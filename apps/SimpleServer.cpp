#include <NetCommon/FwNet.h>

#include <iostream>
#include <memory>

enum class CustomeMsgTypes : uint32_t
{
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll,
    ServerMessage,
};

class CustomServer : public fw::net::ServerInterface<CustomeMsgTypes>
{
  public:
    CustomServer(uint16_t port) : fw::net::ServerInterface<CustomeMsgTypes>{ port }
    {
    }

  protected:
    virtual bool
    on_client_connect(std::shared_ptr<fw::net::Connection<CustomeMsgTypes>> client)
    {
        return true;
    }

    virtual void on_client_disconnect(
        std::shared_ptr<fw::net::Connection<CustomeMsgTypes>> client)
    {
    }

    virtual void
    on_message(std::shared_ptr<fw::net::Connection<CustomeMsgTypes>> client,
               fw::net::Message<CustomeMsgTypes> msg)
    {
    }
};

int main(){
    CustomServer server{60000};
    server.start();

    while(true)
    {
        server.update();
    }
    return 0;
}