#include <NetCommon/FwNet.h>

#include <iostream>

enum class CustomMsgTypes : uint32_t
{
    FireBullet,
    MovePlayer
};

class CustomClient : public fw::net::ClientInterface<CustomMsgTypes>
{
  public:
    bool fire_bullet(float x, float y)
    {
        fw::net::Message<CustomMsgTypes> msg;
        msg.header.id = CustomMsgTypes::FireBullet;
        msg << x << y;
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
    client.connect("community.onelonecoder.com", 60000);
    client.fire_bullet(0.5f, 0.7f);

    return 0;
}