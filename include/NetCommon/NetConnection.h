#pragma once

#include "NetConnection.h"
#include "NetServer.h"
#include "NetTsQueue.h"
#include "NetMessage.h"
#include "NetCommon.h"

namespace fw
{
namespace net
{
// Forward declare
template <typename T> class ServerInterface;

template <typename T>
class Connection : public std::enable_shared_from_this<Connection<T>>
{
  public:
    enum class Owner
    {
        server,
        client,
    };

    Connection(Owner parent, asio::io_context& asio_context,
               asio::ip::tcp::socket socket, TsQueue<OwnedMessage<T>>& message_in)
        : asio_context_{ asio_context }, socket_{ std::move(socket) },
          messages_in_{ message_in }
    {
        owner_type_ = parent;

        // Construct validation check data
        if (owner_type_ == Owner::server)
        {
            // Connection is Server -> Client, construct a random data for the client
            // to transform and send back for validation
            hand_shake_out_ = uint64_t(
                std::chrono::system_clock::now().time_since_epoch().count());

            // Pre-calculate the result for checking when the client responds
            hand_shake_check_ = scramble(hand_shake_out_);
        }
        else
        {
            // Connection is Client -> Server, so we have nothing to define
            hand_shake_in_  = 0;
            hand_shake_out_ = 0;
        }
    }

    virtual ~Connection() {}

    uint32_t id() const { return id_; }

  public:
    // For server only
    void connect_to_client(fw::net::ServerInterface<T>* server, uint32_t id = 0)
    {
        if (owner_type_ == Owner::server)
        {
            if (socket_.is_open())
            {
                id_ = id;
                // was: read_header();

                // A client has attempted to connect to the server, but we wish
                // the client to 1st validate itself, so 1st write out the handshake
                // data to be validated
                write_validation();

                // Next, issue a task to sit and wait asynchronously for precisely
                // the validation data sent back from the client
                read_validation(server);
            }
        }
    }

    void connect_to_server(const asio::ip::tcp::resolver::results_type& endpoints)
    {
        // Only clients can connect to servers
        if (owner_type_ == Owner::client)
        {
            // Request asio attempts to connect to an endpoint
            asio::async_connect(
                socket_,
                endpoints,
                [this](std::error_code ec, asio::ip::tcp::endpoint endpoint)
                {
                    if (!ec)
                    {
                        // Was: read_header();
                        // 1st thing server will do is send packet to be validated so
                        // wait for that and respond
                        read_validation();
                    }
                });
        }
    }
    void disconnect()
    {
        if (is_connected())
        {
            // Close the socket, when it's appropriate for asio to do so
            asio::post(asio_context_, [this] { socket_.close(); });
        }
    }
    bool is_connected() const { return socket_.is_open(); };

  public:
    void send(const Message<T>& msg)
    {
        // injecting more work into an asio context
        asio::post(asio_context_,
                   [this, msg]
                   {
                       bool writing_message{
                           !messages_out_.empty()
                       }; // we can assert whenever asio is already busy doing
                          // writing messages
                       messages_out_.push_back(msg);
                       if (!writing_message)
                       {
                           write_header(); //...or we need to restart that write
                                           // messaging process
                       }
                   });
    }

  private:
    // ASYNC - Prime context ready to read a message header
    void read_header()
    {
        asio::async_read(
            socket_,
            asio::buffer(&msg_temp_in_.header, sizeof(MessageHeader<T>)),
            [this](std::error_code ec, std::size_t length)
            {
                if (!ec)
                {
                    // If we have bodies for out messages
                    if (msg_temp_in_.header.size > 0)
                    {
                        // ...it does, so allocate enough space in the messages' body
                        // vector, and issue asio with the task to read the body.
                        msg_temp_in_.body.resize(msg_temp_in_.header.size);
                        read_body();
                    }
                    else
                    {
                        // it doesn't, so add this bodyless message to the
                        // connections incoming message queue
                        add_to_incoming_message_queue();
                    }
                }
                else
                {
                    // Reading form the client went wrong, most likely a disconnect
                    // has occurred. Close the socket and let the system tidy it up
                    // later.
                    std::cout << "[" << id_ << "] Read Header Fail.\n";
                    socket_.close();
                }
            });
    }

    // ASYNC - Prime context ready to read a message body
    void read_body()
    {
        asio::async_read(
            socket_,
            asio::buffer(msg_temp_in_.body.data(), msg_temp_in_.body.size()),
            [this](std::error_code ec, std::size_t length)
            {
                if (!ec)
                {
                    add_to_incoming_message_queue();
                }
                else
                {
                    // As above!
                    std::cout << "[" << id_ << "] Read Body Fail.\n";
                    socket_.close();
                }
            });
    }

    // ASYNC - Prime context ready to write a message header
    void write_header()
    {
        asio::async_write(
            socket_,
            asio::buffer(&messages_out_.front().header, sizeof(MessageHeader<T>)),
            [this](std::error_code ec, std::size_t length)
            {
                if (!ec)
                {
                    // If we have bodies for out messages
                    if (messages_out_.front().body.size() > 0)
                    {
                        write_body();
                    }
                    else
                    {
                        messages_out_.pop_front();

                        if (!messages_out_.empty())
                        {
                            write_header();
                        }
                    }
                }
                else
                {
                    std::cout << "[" << id_ << "] Write Header Fail.\n";
                    socket_.close();
                }
            });
    }

    // ASYNC - Prime context ready to write a message body
    void write_body()
    {
        asio::async_write(socket_,
                          asio::buffer(messages_out_.front().body.data(),
                                       messages_out_.front().body.size()),
                          [this](std::error_code ec, size_t length)
                          {
                              if (!ec)
                              {
                                  messages_out_.pop_front();

                                  if (!messages_out_.empty())
                                  {
                                      write_header();
                                  }
                              }

                              else
                              {
                                  std::cout << "[" << id_ << "] Write Body Fail.\n";
                                  socket_.close();
                              }
                          });
    }

    void add_to_incoming_message_queue()
    {
        if (owner_type_ == Owner::server)
        {
            messages_in_.push_back({ this->shared_from_this(), msg_temp_in_ });
        }
        else
        {
            messages_in_.push_back({ nullptr, msg_temp_in_ });
        }

        read_header();
    }

    // "Encrypt" data
    uint64_t scramble(uint64_t input)
    {
        uint64_t out{ input ^ 0xDEADBEEFC0DECAFE };
        out = (out & 0xF0F0F0F0F0F0F0) >> 4 | (out & 0xF0F0F0F0F0F0F0) << 4;
        return out ^ 0xC0DEFACE12345678;
    }

    // ASYNC - Used by both client and server to write validation packet
    void write_validation()
    {
        asio::async_write(socket_,
                          asio::buffer(&hand_shake_out_, sizeof(uint64_t)),
                          [this](std::error_code ec, size_t lenght)
                          {
                              if (!ec)
                              {
                                  // Validation data sent, clients should sit and
                                  // wait for a response (or a closure)
                                  if (owner_type_ == Owner::client)
                                  {
                                      read_header();
                                  }
                              }
                              else
                              {
                                  socket_.close();
                              } 
                          });
    }

    void read_validation(fw::net::ServerInterface<T>* server = nullptr)
    {
        asio::async_read(
            socket_,
            asio::buffer(&hand_shake_in_, sizeof(uint64_t)),
            [this, server](std::error_code ec, size_t length)
            {
                if (!ec)
                {
                    if (owner_type_ == Owner::server)
                    {
                        // Client has provided valid solution, so allow it to connect
                        if (hand_shake_in_ == hand_shake_check_)
                        {
                            std::cout << "Client validated" << std::endl;
                            server->on_client_validated(this->shared_from_this());

                            // Sit waiting to receive data now
                            read_header();
                        }

                        else
                        {
                            // Client gave incorrect data, so disconnect
                            std::cout << "Client disconnected (Fail Validation)"
                                      << std::endl;
                            socket_.close();
                        }
                    }
                    else
                    {
                        // Connection is a client, so solve the puzzle
                        hand_shake_out_ = scramble(hand_shake_in_);

                        // write the result
                        write_validation();
                    }
                }
                else
                {
                    // Some failure occured
                    std::cout << "Client Disconnected (read_validation)"
                              << std::endl;
                    socket_.close();
                }
            });
    }

  protected:
    // Each connection has unique socket to a remote
    asio::ip::tcp::socket socket_;

    // This context is shared with the whole asio instance
    asio::io_context& asio_context_;

    // This queue holds all messages to be send to the remote side of this connection
    TsQueue<Message<T>> messages_out_;

    // This queue holds all messages that have been received from the remote side of
    // this connection. Note it is a referece as the "owner" of this connection is
    // expected to provide a queue
    TsQueue<OwnedMessage<T>>& messages_in_;

    Message<T> msg_temp_in_;

    // The "owner" decides how some of the connection behaves
    Owner owner_type_{ Owner::server };
    uint32_t id_{ 0 };

    // Handshake validation
    uint64_t hand_shake_out_{ 0 };
    uint64_t hand_shake_in_{ 0 };
    uint64_t hand_shake_check_{ 0 };
};
} // namespace net

} // namespace fw
