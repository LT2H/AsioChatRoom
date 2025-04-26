#include "NetCommon/NetMessage.h"
#include <NetCommon/FwNet.h>

#include <array>
#include <cstring>
#include <iostream>
#include <memory>
#include <minwindef.h>

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
        fw::net::Message<CustomMsgTypes> msg;
        msg.header.id = CustomMsgTypes::ServerAccept;
        client->send(msg);
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

        case CustomMsgTypes::ServerMessage:
        {
            std::cout << "[" << client->id() << "]: Server Message\n";

            std::array<char, fw::net::array_size> rep{};
            std::array<char, fw::net::array_size> data{
                "I am server, msg back to you"
            };
            rep = data;
            msg << rep;

            client->send(msg);
        }
        break;

        case CustomMsgTypes::MessageAll:
        {
            std::cout << "[" << client->id() << "]: Message All\n";
            fw::net::Message<CustomMsgTypes> sending_message{};
            sending_message.header.id = CustomMsgTypes::ServerMessage;

            std::array<char, fw::net::array_size> msg_content{};
            msg >> msg_content;

            // Must reverse the order
            sending_message << msg_content;
            sending_message << client->id();

            // Send the message to all clients
            message_all_clients(sending_message, client);
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
        server.update(65535, true);
    }
    return 0;
}