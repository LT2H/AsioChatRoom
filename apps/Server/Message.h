#pragma once

#include "ClientInfo.h"

#include <NetCommon/NetCommon.h>

struct Message
{
    ClientInfo client_info;
    std::array<char, net::array_size> content;
};