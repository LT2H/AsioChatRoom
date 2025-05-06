#pragma once

#include <NetCommon/FwNet.h>

enum class CustomMsgTypes : uint32_t
{
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll,
    ServerMessage,
    NewClientConnected,
    ClientDisconnected,
    ACKOtherClients,
};