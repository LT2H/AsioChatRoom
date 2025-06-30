#pragma once

#include <NetCommon/NetCommon.h>
#include "../CustomMsgTypes.h"

struct ClientInfo
{
    std::shared_ptr<net::Connection<CustomMsgTypes>> conn;
    std::array<char, net::array_size> name{ "Anon" };

    std::array<float, 3> color{ 1.0f, 1.0f, 1.0f };

    std::string to_string() const { return std::string{ name.data() }; }
};