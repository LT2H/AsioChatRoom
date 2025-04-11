#pragma once

#include "NetConnection.h"
#include "NetTsQueue.h"
#include "NetMessage.h"
#include "NetCommon.h"
#include "asio/io_context.hpp"
#include <cstdint>

namespace fw
{
namespace net
{
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
    }

    virtual ~Connection() {}

    uint32_t id() const { return id_; }

  public:
    // For server only
    void connect_to_client(uint32_t id = 0)
    {
        if (owner_type_ == Owner::server)
        {
            if (socket_.is_open())
            {
                id_ = id;
            }
        }
    }

    bool connect_to_server();
    bool disconnect();
    bool is_connected() const { return socket_.is_open();  };

  public:
    bool send(const Message<T>& msg);

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

    // The "owner" decides how some of the connection behaves
    Owner owner_type_{ Owner::server };
    uint32_t id_{ 0 };
};
} // namespace net

} // namespace fw
