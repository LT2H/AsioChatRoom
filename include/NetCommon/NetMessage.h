#pragma once

#include "NetCommon.h"

namespace net
{
// Message Header is sent at start of all messages. The template allow us
// to use "enum class" to ensure the messagses are valid at compile time
template <typename T> struct MessageHeader
{
    T id{};
    uint32_t size{ 0 }; // Size of the whole message
};

template <typename T> struct Message
{
    MessageHeader<T> header{};

    std::vector<uint8_t> body;
    // returns the size of entire message packet in bytes
    size_t size() const { return body.size(); }

    // Override for std::cout compatibility - produces friendly description of
    // message
    friend std::ostream& operator<<(std::ostream& os, const Message<T>& msg)
    {
        os << "Id:" << static_cast<int>(msg.header.id) << "Size:" << msg.header.size;
        return os;
    }

    // Pushes any POD-like data into the message buffer
    template <typename DataType>
    friend Message<T>& operator<<(Message<T>& msg, const DataType& data)
    {
        // Check that the type of the data being pushed is trivially copyable
        static_assert(std::is_standard_layout<DataType>::value,
                      "Data is too complex to be pushed into vector");

        // Cache current size of vector, as this will be the point we insert the data
        size_t i{ msg.body.size() };

        // Resize the vector by the size of the data being pushed
        msg.body.resize(msg.body.size() + sizeof(DataType));

        // Physically copy the data into the newly allocated vector space
        std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

        // Recalculate the message size
        msg.header.size = msg.size();

        // Return the target message so it can be chained
        return msg;
    }

    template <typename DataType>
    friend Message<T>& operator>>(Message<T>& msg, DataType& data)
    {
        // Check that the type of the data being pushed is trivially copyable
        static_assert(std::is_standard_layout<DataType>::value,
                      "Data is too complex to be pulled from vector");

        if (msg.body.size() < sizeof(DataType))
            throw std::runtime_error("Not enough data in message to extract");

        // Cache the location towards the end of the vector where the pulled data
        // starts
        size_t i{ msg.body.size() - sizeof(DataType) };

        // Physically copy the data from the vector into the user variable
        std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

        // Shrink the vector to remove the read bytes, and reset end pos
        msg.body.resize(i);

        // Recalculate the message size
        msg.header.size = msg.size();

        // Return the target message so it can be chained
        return msg;
    }
};

} // namespace net
