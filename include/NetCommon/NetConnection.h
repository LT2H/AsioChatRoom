#pragma once

#include "NetConnection.h"
#include "NetTsQueue.h"
#include "NetMessage.h"
#include "NetCommon.h"

namespace olc
{
namespace net
{
template <typename T>
class Connection : public std::enable_shared_from_this<Connection<T>>
{
  public:
    Connection() {}

    virtual ~Connection() {}

  public:
    bool connect_to_server();
    bool disconnect();
    bool is_connected() const;

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
    TsQueue<OwnedMessage>& messages_in_;
};
} // namespace net

} // namespace olc
