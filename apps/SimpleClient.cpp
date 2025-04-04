#include <NetCommon/NetMessage.h>

#include <iostream>

enum class CustomMsgTypes : uint32_t
{
    FireBullet,
    MovePlayer
};

int main()
{
    olc::net::Message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::FireBullet;

    int a{ 1 };
    bool b{ true };
    float c{ 3.14159f };

    struct Data
    {
        float x;
        float y;
    };

    std::array<Data, 5> d{};

    msg << a << b << c << d;
    
    a = 99;
    b = false;
    c = 99.0f;

    msg >> d >> c >> b >> a;

    return 0;
}