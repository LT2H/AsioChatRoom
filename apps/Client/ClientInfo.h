#pragma once

#include <NetCommon/FwNet.h>

struct ClientInfo
{
    uint32_t id{ 0 };
    std::array<char, fw::net::array_size> name{ "Anon" };

    std::array<float, 3> color{ 1.0f, 1.0f, 1.0f };

    constexpr std::string to_string() const { return std::string{ name.data() }; }
};