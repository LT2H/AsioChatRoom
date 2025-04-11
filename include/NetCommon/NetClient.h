#pragma once

#include "NetConnection.h"
#include "NetTsQueue.h"
#include "NetMessage.h"
#include "NetCommon.h"

#include <string_view>

namespace fw
{
namespace net
{
template <typename T> class ClientInterface
{
  public:
    ClientInterface() : socket_{ context_ }
    { // Initialise the socket with the io context, so it can do stuff
    }

    virtual ~ClientInterface()
    {
        // If the client is destroyed, always try and disconnect from server
        disconnect();
    }

    // Connect to server with hostname/ip-address and port
    bool connect(std::string_view host, const uint16_t port)
    {
        try
        {
            // Create connection
            connection_ = std::make_unique<Connection<T>>(); // TODO

            // Resolve hostname/ip-address into tangiable physical address
            asio::ip::tcp::resolver resolver{ context_ };
            asio::ip::tcp::resolver::results_type endpoints =
                resolver.resolve(host, std::to_string(port));

            // Tell the connection object tp connect to server
            connection_->connect_to_server(endpoints);

            // Start context thread
            thr_context_ = std::thread([this] { context_.run(); });
        }
        catch (std::exception& e)
        {
            std::cerr << "Client Exception: " << e.what() << "\n";
        }
        return false;
    }

    // Disconnect from server
    void disconnect()
    {
        if (is_connected())
        {
            connection_->disconnect();

            // Either way, we're also done with the asio context...
            context_.stop();
            //...and its thread
            if (thr_context_.joinable())
            {
                thr_context_.join();
            }

            connection_.release();
        }
    }

    // Check if client is actually connected to a server
    bool is_connected()
    {
        if (connection_)
            return connection_->is_connected();
        else
            return false;
    }

    void send(const Message<T>& msg) {
        if (is_connected()) {
            connection_->send(msg);
        }
    }

    // Retrieve queue of messages from server
    TsQueue<OwnedMessage<T>>& incoming() { return messages_in_; }

  protected:
    // asio context handles the data transfer...
    asio::io_context context_;

    //...but needs a thread of its own to execute its work commands
    std::thread thr_context_;

    // This is the hardware socket that is connected to the server
    asio::ip::tcp::socket socket_;

    // The client has a single instance of a "connection object", which handles
    // data transfer
    std::unique_ptr<Connection<T>> connection_;

  private:
    // This is the thread safe queue of incoming messages from server;
    TsQueue<OwnedMessage<T>> messages_in_;
};

} // namespace net
} // namespace fw
