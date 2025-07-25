#include "Message.h"

#include "NetCommon/NetMessage.h"
#include <NetCommon/NetCommon.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

class Server : public net::ServerInterface<CustomMsgTypes>
{
  public:
    Server(uint16_t port) : net::ServerInterface<CustomMsgTypes>{ port } {}

  protected:
    virtual bool
    on_client_connect(std::shared_ptr<net::Connection<CustomMsgTypes>> client)
    {
        net::Message<CustomMsgTypes> msg;
        msg.header.id = CustomMsgTypes::ServerAccept;
        client->send(msg);

        //  The new client should also know about all previous clients
        for (auto& [id, existing_client] : clients_)
        {
            if (existing_client.conn != client)
            {
                net::Message<CustomMsgTypes> msg_about_existing_client;
                msg_about_existing_client.header.id =
                    CustomMsgTypes::ACKOtherClients;

                msg_about_existing_client << existing_client.color
                                          << existing_client.name << id;

                client->send(msg_about_existing_client);
            }
        }

        return true;
    }

    virtual void
    on_client_disconnect(std::shared_ptr<net::Connection<CustomMsgTypes>> client)
    {
    }

    virtual void on_message(std::shared_ptr<net::Connection<CustomMsgTypes>> client,
                            net::Message<CustomMsgTypes>& msg)
    {
        switch (msg.header.id)
        {
        case CustomMsgTypes::NewClientConnected:
        {
            std::cout << "[" << client->id() << "]: New Client Connected\n";

            net::Message<CustomMsgTypes> msg_for_other_clients{};
            msg_for_other_clients.header.id = CustomMsgTypes::NewClientConnected;

            ClientInfo new_client_info{};

            msg >> new_client_info.color >> new_client_info.name;

            // Track this client
            clients_[client->id()] = new_client_info;

            // Must reverse the order
            msg_for_other_clients << new_client_info.color << new_client_info.name
                                  << client->id();

            message_all_clients(msg_for_other_clients, client);
        }
        break;

        case CustomMsgTypes::ClientDisconnected:
        {
            std::cout << "[" << client->id() << "]: Client Disconnected\n";

            clients_.erase(client->id());

            net::Message<CustomMsgTypes> msg_for_other_clients{};
            msg_for_other_clients.header.id = CustomMsgTypes::ClientDisconnected;
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

            std::array<char, net::array_size> rep{};
            std::array<char, net::array_size> data{ "I am server, msg back to you" };
            rep = data;
            msg << rep;

            client->send(msg);
        }
        break;

        case CustomMsgTypes::MessageAll:
        {
            std::cout << "[" << client->id() << "]: Message All\n";
            net::Message<CustomMsgTypes> sending_message{};
            sending_message.header.id = CustomMsgTypes::ServerMessage;

            Message message{};
            msg >> message.client_info.name >> message.client_info.color >>
                message.content;

            // Must reverse the order
            sending_message << message.content << message.client_info.color
                            << message.client_info.name << client->id();

            // Send the message to all clients
            message_all_clients(sending_message, client);
        }
        break;
        }
    }

  private:
    std::unordered_map<uint32_t, ClientInfo> clients_{};
};

bool clear_failed_extraction()
{
    if (!std::cin)
    {
        if (std::cin.eof()) // If the stream was closed
        {
            std::exit(0);
        }

        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Remove the bad input
        return true;
    }
    return false;
}

uint16_t promt_port()
{
    while (true)
    {
        std::cout << "Enter the port you would like the server to use: ";
        int port{};
        std::cin >> port;

        if (clear_failed_extraction())
        {
            std::cout << "Invalid port. Please try again\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Remove the bad input
        return port;
    }
}

int main()
{
    Server server{ promt_port() };

    server.start();

    while (true)
    {
        server.update(65535, true);
    }
    return 0;
}