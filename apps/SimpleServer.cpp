#include <NetCommon/FwNet.h>

#include <iostream>
#include <memory>

enum class CustomMsgTypes : uint32_t
{
    ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
};

class CustomServer : public fw::net::ServerInterface<CustomMsgTypes>
{
  public:
    CustomServer(uint16_t port) : fw::net::ServerInterface<CustomMsgTypes>{ port } {}

  protected:
    virtual bool
    on_client_connect(std::shared_ptr<fw::net::Connection<CustomMsgTypes>> client)
    {
        return true;
    }

    virtual void
    on_client_disconnect(std::shared_ptr<fw::net::Connection<CustomMsgTypes>> client)
    {
    }

    virtual void
    on_message(std::shared_ptr<fw::net::Connection<CustomMsgTypes>> client,
               fw::net::Message<CustomMsgTypes>& msg)
    {
        switch (msg.header.id)
        {
        case CustomMsgTypes::ServerPing:
        {
            std::cout << "[" << client->id() << "]: Server Ping\n";

            // Simply bounce message back to client
            client->send(msg);
        }
        break;
        }
    }
};

int main()
{
    CustomServer server{ 60000 };
    server.start();

    while (true)
    {
        server.update();
    }
    return 0;
}