#include <mylib/lib.h>

#include <iostream>

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

int main()
{
    asio::error_code ec;

    // Create a "context" - essentially the platform specific interface
    asio::io_context context;

    // Get the address of somewhere we wish to connect to
    asio::ip::tcp::endpoint endpoint{ asio::ip::make_address("8.8.8.8", ec), 53 };

    // Create a socket, the context will deliver the impl
    asio::ip::tcp::socket socket{ context };

    // Tell socket to try and connect
    socket.connect(endpoint, ec);

    if (!ec)
    {
        std::cout << "Connected" << std::endl;
    }
    else
    {
        std::cout << "Failed to connect to address:\n" << ec.message() << "\n";
    }

    std::cin.get();
    return 0;
}
