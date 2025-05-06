#pragma once

#include "ClientInfo.h"

#include <NetCommon/FwNet.h>

struct Message
{
    ClientInfo client_info;
    std::array<char, fw::net::array_size> content;
};