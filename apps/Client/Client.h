#pragma once

#include "../CustomMsgTypes.h"
#include "ClientInfo.h"
#include "Message.h"

#include "NetCommon/NetMessage.h"
#include <NetCommon/NetCommon.h>

#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

class Client : public net::ClientInterface<CustomMsgTypes>
{
  public:
    ~Client() override
    {
        net::Message<CustomMsgTypes> disconnect_msg{};
        disconnect_msg.header.id = CustomMsgTypes::ClientDisconnected;
        send(disconnect_msg);
        disconnect();
    }
    void ping_server()
    {
        net::Message<CustomMsgTypes> msg;
        msg.header.id = CustomMsgTypes::ServerPing;

        // Caution with this
        std::chrono::system_clock::time_point time_now{
            std::chrono::system_clock::now()
        };

        msg << time_now;

        send(msg);
    }

    void message_all()
    {
        net::Message<CustomMsgTypes> msg;
        msg.header.id = CustomMsgTypes::MessageAll;

        msg << sending_msg_.content << sending_msg_.client_info.color
            << sending_msg_.client_info.name;

        send(msg);
    }

    void set_sending_msg(Message&& sending_msg)
    {
        sending_msg.client_info = info_;
        sending_msg_            = sending_msg;

        // Your message
        append_msg(sending_msg_);
    }

    Message sending_msg() const { return sending_msg_; }

    std::vector<Message> messages() const { return messages_; }

    // For the Ui only
    void append_msg(const Message& message) { messages_.push_back(message); }

    void handle_incoming_msg()
    {
        if (is_connected())
        {
            if (!incoming().empty())
            {
                auto msg{ incoming().pop_front().msg };
                switch (msg.header.id)
                {
                case CustomMsgTypes::ServerAccept:
                {
                    std::cout << "Server Accepted Connection\n";

                    ClientInfo this_client_info{0, info_.name, info_.color};

                    clients_list_.push_back(this_client_info);

                    net::Message<CustomMsgTypes> broadcasting_msg{};
                    broadcasting_msg.header.id = CustomMsgTypes::NewClientConnected;
                    broadcasting_msg << info_.name; // with this client's name
                    broadcasting_msg << info_.color;

                    send(broadcasting_msg);
                }
                break;

                case CustomMsgTypes::ACKOtherClients:
                {
                    ClientInfo client_info{};

                    try
                    {
                        msg >> client_info.id >> client_info.name >>
                            client_info.color;
                    }
                    catch (const std::runtime_error& e)
                    {
                        std::cerr << e.what() << "\n";
                    }

                    clients_list_.emplace_back(client_info);
                }
                break;

                case CustomMsgTypes::NewClientConnected:
                {
                    ClientInfo client_info{};

                    try
                    {
                        msg >> client_info.id >> client_info.name >>
                            client_info.color;
                    }
                    catch (const std::runtime_error& e)
                    {
                        std::cerr << e.what() << "\n";
                    }

                    clients_list_.emplace_back(client_info);
                }
                break;

                case CustomMsgTypes::ClientDisconnected:
                {
                    uint32_t client_id_to_remove{};
                    msg >> client_id_to_remove;

                    clients_list_.erase(
                        std::remove_if(
                            clients_list_.begin(),
                            clients_list_.end(),
                            [client_id_to_remove](const ClientInfo& client)
                            { return client.id == client_id_to_remove; }),
                        clients_list_.end());
                }
                break;

                case CustomMsgTypes::ServerPing:
                {
                    std::chrono::system_clock::time_point time_now{
                        std::chrono::system_clock::now()
                    };
                    std::chrono::system_clock::time_point time_then;
                    msg >> time_then;

                    std::cout << "Ping: "
                              << std::chrono::duration<double>(time_now - time_then)
                                     .count()
                              << "\n";
                }
                break;

                case CustomMsgTypes::ServerMessage:
                {
                    // uint32_t clientID{};
                    // std::array<char, net::array_size> msg_content{};

                    Message message{};

                    try
                    {
                        msg >> message.client_info.id >> message.client_info.name >>
                            message.client_info.color >> message.content;
                    }
                    catch (const std::runtime_error& e)
                    {
                        std::cerr << e.what() << "\n";
                    }

                    append_msg(message);
                }
                break;
                }
            }
        }
    }

    std::array<char, net::array_size>& name() { return info_.name; }

    std::array<float, 3>& color() { return info_.color; }

    std::array<float, 3> color() const { return info_.color; }

    std::vector<ClientInfo> clients_list() const { return clients_list_; }

    constexpr std::array<char, net::array_size> name() const
    {
        return info_.name;
    }

    void clear_clients_list() { clients_list_.clear(); }

  private:
    ClientInfo info_{};

    Message sending_msg_{};
    std::vector<Message> messages_;
    std::vector<ClientInfo> clients_list_{};
};
