#pragma once

#include "NetCommon/NetMessage.h"
#include <NetCommon/FwNet.h>

#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <windows.h>

enum class CustomMsgTypes : uint32_t
{
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll,
    ServerMessage,
    NewClientConnected,
    ClientDisconnected,
    ACKOtherClients
};

struct ClientInfo
{
    uint32_t id{ 0 };
    std::array<char, fw::net::array_size> name{ "(unknown)" };

    std::string to_string() const { return std::string{ name.data() }; }
};

class CustomClient : public fw::net::ClientInterface<CustomMsgTypes>
{
  public:
    ~CustomClient() override
    {
        fw::net::Message<CustomMsgTypes> disconnect_msg{};
        disconnect_msg.header.id = CustomMsgTypes::ClientDisconnected;
        send(disconnect_msg);
        disconnect();
    }
    void ping_server()
    {
        fw::net::Message<CustomMsgTypes> msg;
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
        fw::net::Message<CustomMsgTypes> msg;
        msg.header.id = CustomMsgTypes::MessageAll;

        msg << sending_msg_;

        send(msg);
    }

    void set_sending_msg(std::array<char, fw::net::array_size>&& sending_msg)
    {
        sending_msg_ = sending_msg;
        append_msg(sending_msg_);
    }

    constexpr std::array<char, fw::net::array_size> sending_msg() const
    {
        return sending_msg_;
    }

    constexpr std::vector<std::array<char, fw::net::array_size>> messages() const
    {
        return messages_;
    }

    void append_msg(const std::array<char, fw::net::array_size>& message)
    {
        messages_.push_back(message);
    }

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

                    fw::net::Message<CustomMsgTypes> broadcasting_msg{};
                    broadcasting_msg.header.id = CustomMsgTypes::NewClientConnected;
                    broadcasting_msg << name_; // with this client's name

                    send(broadcasting_msg);
                }
                break;

                case CustomMsgTypes::ACKOtherClients:
                {
                    uint32_t client_id{};
                    std::array<char, fw::net::array_size> name{};

                    try
                    {
                        msg >> client_id;
                        msg >> name;
                    }
                    catch (const std::runtime_error& e)
                    {
                        std::cerr << e.what() << "\n";
                    }

                    other_clients_list_.emplace_back(client_id, name);
                }
                break;

                case CustomMsgTypes::NewClientConnected:
                {
                    uint32_t client_id{};
                    std::array<char, fw::net::array_size> name{};

                    try
                    {
                        msg >> client_id;
                        msg >> name;
                    }
                    catch (const std::runtime_error& e)
                    {
                        std::cerr << e.what() << "\n";
                    }

                    other_clients_list_.emplace_back(client_id, name);
                }
                break;

                case CustomMsgTypes::ClientDisconnected:
                {
                    uint32_t client_id_to_remove{};
                    msg >> client_id_to_remove;

                    other_clients_list_.erase(
                        std::remove_if(
                            other_clients_list_.begin(),
                            other_clients_list_.end(),
                            [client_id_to_remove](const ClientInfo& client)
                            { return client.id == client_id_to_remove; }),
                        other_clients_list_.end());
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
                    uint32_t clientID{};
                    std::array<char, fw::net::array_size> msg_content{};

                    try
                    {
                        msg >> clientID;
                        msg >> msg_content;
                    }
                    catch (const std::runtime_error& e)
                    {
                        std::cerr << e.what() << "\n";
                    }


                    std::cout << "[" << clientID << "] Said : " << msg_content.data()
                              << "\n";

                    append_msg(msg_content);
                }
                break;
                }
            }
        }
    }

    void set_name(const std::array<char, fw::net::array_size>& name)
    {
        name_ = name;
    }

    constexpr std::vector<ClientInfo> other_clients_list() const
    {
        return other_clients_list_;
    }

    constexpr std::array<char, fw::net::array_size> name() const { return name_; }

  private:
    std::array<char, fw::net::array_size> name_{};

    std::array<char, fw::net::array_size> sending_msg_{};
    std::vector<std::array<char, fw::net::array_size>> messages_;
    std::vector<ClientInfo> other_clients_list_{};
};
