#pragma once

#include "NetCommon.h"
#include "NetTsQueue.h"
#include "NetMessage.h"
#include "NetConnection.h"

namespace net
{
template <typename T> class ServerInterface
{
  public:
    ServerInterface(uint16_t port)
        : asio_acceptor_{ asio_context_,
                          asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port) }
    {
    }

    virtual ~ServerInterface() { stop(); }

    bool start()
    {
        try
        {
            // Issue a task to the asio context - This
            // is important
            // as it will prime the context with "work", and stop it
            // from exiting immediately. Since this is a server, we
            // want it primed ready to handle clients trying to
            // connect.
            wait_for_client_connection();

            thread_context_ = std::thread([this] { asio_context_.run(); });
        }
        catch (const std::exception& e)
        {
            std::cerr << "[SERVER] Exception: " << e.what() << '\n';
            return false;
        }

        std::cout << "[SERVER] Started!\n";
        return true;
    }

    void stop()
    {
        // Request the context to close
        asio_context_.stop();

        // Tidy up the context thread
        if (thread_context_.joinable())
        {
            thread_context_.join();
        }

        // Inform someone, anybody
        std::cout << "[SERVER] Stopped!\n";
    }

    // ASYNC - Instruct asio to wait for connection
    void wait_for_client_connection()
    {
        asio_acceptor_.async_accept(
            [this](std::error_code ec, asio::ip::tcp::socket socket)
            {
                if (!ec)
                {
                    std::cout
                        << "[SERVER] New Connection: " << socket.remote_endpoint()
                        << "\n";

                    auto new_conn{ std::make_shared<Connection<T>>(
                        Connection<T>::Owner::server,
                        asio_context_,
                        std::move(socket),
                        messages_in_) };

                    // Give the user server a chance to deny connection
                    if (on_client_connect(new_conn))
                    {
                        // Connection allowed, so add to container of new conns
                        connections_.push_back(std::move(new_conn));

                        connections_.back()->connect_to_client(this,
                                                               ++client_id_counter_);

                        std::cout << "[" << connections_.back()->id()
                                  << "] Connection Approved\n";
                    }
                    else
                    {
                        std::cout << "[-----] Connection Denied\n";
                    }
                }
                else
                {
                    // Error has occurred during acceptance
                    std::cout << "[SERVER] New Connection Error: " << ec.message()
                              << "\n";
                }

                // Prime the asio context with more work - again simply wait for
                // another connection...
                wait_for_client_connection();
            });
    }

    // Send a message to a specific client
    void message_client(std::shared_ptr<Connection<T>> client, const Message<T>& msg)
    {
        if (client && client->is_connected())
        {
            client->send(msg);
        }
        else
        {
            on_client_disconnect(client);
            client.reset();
            connections_.erase(
                std::remove(connections_.begin(), connections_.end(), client),
                connections_.end()); // expensive if we have many clients connected
        }
    }

    // Send messag to all clients
    void message_all_clients(const Message<T>& msg,
                             std::shared_ptr<Connection<T>> ignored_client = nullptr)
    {
        bool invalid_client_exists{ false };

        for (auto& client : connections_)
        {
            // Check client is connected...
            if (client && client->is_connected())
            {
                if (client != ignored_client)
                {
                    client->send(msg);
                }
            }
            else
            {
                // The client couldnt be contacted, so assume it has disconnected
                on_client_disconnect(client);
                client.reset();
                invalid_client_exists = true;
            }
        }

        if (invalid_client_exists)
        {
            connections_.erase(
                std::remove(connections_.begin(), connections_.end(), nullptr),
                connections_.end());
        }
    }

    void update(size_t max_messages = 65535, bool wait = false)
    {
        if (wait)
            messages_in_.wait();

        size_t message_count{ 0 };
        while (message_count < max_messages && !messages_in_.empty())
        {
            // Grab the front message
            auto msg{ messages_in_.pop_front() };

            // Pass to message handler
            on_message(msg.remote, msg.msg);

            ++message_count;
        }
    }

  protected:
    // Called when a client connects, you can veto the connection by returning false
    virtual bool on_client_connect(std::shared_ptr<Connection<T>> client)
    {
        return false;
    }

    // Called when a client appears to have disconnected
    virtual void on_client_disconnect(std::shared_ptr<Connection<T>> client) {}

    // Called when a message arrives
    virtual void on_message(std::shared_ptr<Connection<T>> client, Message<T>& msg)
    {
    }

  public:
    // Called when a client is validated
    virtual void on_client_validated(std::shared_ptr<Connection<T>> client) {}

  protected:
    // Thread Safe Queue for incoming message packets
    TsQueue<OwnedMessage<T>> messages_in_;

    // Container of active validated connections
    std::deque<std::shared_ptr<Connection<T>>> connections_;

    // Order of declaration is important - it is also the order of initialisation
    asio::io_context asio_context_;
    std::thread thread_context_;

    // These things need an asio context
    asio::ip::tcp::acceptor asio_acceptor_;

    // Clients will be identified in the "wider system" via an ID
    uint32_t client_id_counter_{ 10000 };
};

} // namespace net