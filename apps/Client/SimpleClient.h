#pragma once

#include "NetCommon/NetMessage.h"
#include <NetCommon/FwNet.h>

#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include <windows.h>

enum class CustomMsgTypes : uint32_t
{
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll,
    ServerMessage,
    NewClientConnected
};

struct ClientInfo
{
    std::array<char, fw::net::array_size> name{ "(unknown)" };

    std::string to_string() const {
        return std::string{name.data()};
    }
};

class CustomClient : public fw::net::ClientInterface<CustomMsgTypes>
{
  public:
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

                    send(broadcasting_msg);
                }
                break;

                case CustomMsgTypes::NewClientConnected:
                {
                    std::cout << "New friend!\n";

                    uint32_t clientID{};

                    msg >> clientID;

                    other_clients_list_.push_back(clientID);
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

    constexpr std::string name() const { return name_; }

    void set_name(std::string_view name) { name_ = name; }

    constexpr std::vector<uint32_t> other_clients_list() const
    {
        return other_clients_list_;
    }

    constexpr ClientInfo info() const { return info_; }

    void set_info(std::array<char, fw::net::array_size> info) { info_.name = info; }

  private:
    ClientInfo info_{};
    std::array<char, fw::net::array_size> sending_msg_{};
    std::vector<std::array<char, fw::net::array_size>> messages_;
    std::vector<uint32_t> other_clients_list_{};
    std::string name_{ "(unknown)" };
};
