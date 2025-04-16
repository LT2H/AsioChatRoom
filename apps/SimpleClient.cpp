#include <NetCommon/FwNet.h>

#include <iostream>
#include <windows.h>
enum class CustomMsgTypes : uint32_t
{
    ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
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

    void send_msg_to_server()
    {
        fw::net::Message<CustomMsgTypes> msg;
        msg.header.id = CustomMsgTypes::ServerMessage;

        msg << "OI SERVER";

        send(msg);
    }
};

int main()
{
    // fw::net::Message<CustomMsgTypes> msg;
    // msg.header.id = CustomMsgTypes::FireBullet;

    // int a{ 1 };
    // bool b{ true };
    // float c{ 3.14159f };

    // struct Data
    // {
    //     float x;
    //     float y;
    // };

    // std::array<Data, 5> d{};

    // msg << a << b << c << d;

    // a = 99;
    // b = false;
    // c = 99.0f;

    // msg >> d >> c >> b >> a;

    CustomClient client;
    client.connect("127.0.0.1", 60000);

    std::array keys{ false, false, false, false};
    std::array old_keys{ false, false, false,false };

    bool is_quit{ false };
    while (!is_quit)
    {
        if (GetForegroundWindow() == GetConsoleWindow())
        {
            keys[0] = GetAsyncKeyState('1') & 0x8000;
            keys[1] = GetAsyncKeyState('2') & 0x8000;
            keys[2] = GetAsyncKeyState('3') & 0x8000;
            keys[3] = GetAsyncKeyState('4') & 0x8000;
        }

        if (keys[0] && !old_keys[0])
        {
            client.ping_server();
        }

        if (keys[3] && !old_keys[3]) {
            client.send_msg_to_server();
        }

        if (keys[2] && !old_keys[2])
        {
            is_quit = true;
        }

        for (int i{ 0 }; i < keys.size(); ++i)
        {
            old_keys[i] = keys[i];
        }

        if (client.is_connected())
        {
            if (!client.incoming().empty())
            {
                auto msg{ client.incoming().pop_front().msg };
                switch (msg.header.id)
                {
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
                }
            }
        }
        else
        {
            std::cout << "Server Down\n";
            is_quit = true;
        }
    }

    return 0;
}