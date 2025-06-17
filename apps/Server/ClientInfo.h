#pragma once

#include <NetCommon/FwNet.h>
#include "../CustomMsgTypes.h"

struct ClientInfo
{
    std::shared_ptr<fw::net::Connection<CustomMsgTypes>> conn;
    std::array<char, fw::net::array_size> name{ "Anon" };

    std::array<float, 3> color{ 1.0f, 1.0f, 1.0f };

    std::string to_string() const { return std::string{ name.data() }; }
};