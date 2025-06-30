#pragma once

#include <NetCommon/NetCommon.h>

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