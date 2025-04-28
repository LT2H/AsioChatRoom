#include "NetCommon/NetMessage.h"
#include <NetCommon/FwNet.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <minwindef.h>
#include <unordered_map>
#include <vector>

enum class CustomMsgTypes : uint32_t
{
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll,
    ServerMessage,
    NewClientConnected,
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
        case CustomMsgTypes::NewClientConnected:
        {
            std::cout << "[" << client->id() << "]: New Client Connected\n";
            
            fw::net::Message<CustomMsgTypes> msg_for_other_clients{};
            msg_for_other_clients.header.id = CustomMsgTypes::NewClientConnected;
            msg_for_other_clients << client->id();

            message_all_clients(msg_for_other_clients, client);
        }
        break;

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

  private:
    std::unordered_map<uint32_t,
                       std::shared_ptr<fw::net::Connection<CustomMsgTypes>>>
        clients_{};
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