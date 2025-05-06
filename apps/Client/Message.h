#pragma once

#include "ClientInfo.h"

#include <NetCommon/FwNet.h>
#include <ctime>

struct Message
{
    ClientInfo client_info;
    std::array<char, fw::net::array_size> content;
    std::time_t datetime{ std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now()) }; // Convert time_point to time_t

    constexpr std::string formatted_time() const
    {
        std::tm* time_info{ std::localtime(&datetime) };
        std::array<char, 64> buffer{};
        std::strftime(buffer.data(), sizeof(buffer), "%Y-%m-%d %H:%M", time_info);
        return std::string{buffer.data()};
    }
};