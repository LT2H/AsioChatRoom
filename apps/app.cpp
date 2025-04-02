#include <mylib/lib.h>

#include <iostream>
#include <vector>

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif
// #define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

int main()
{
    asio::error_code ec;

    // Create a "context" - essentially the platform specific interface
    asio::io_context context;

    // Get the address of somewhere we wish to connect to
    // asio::ip::tcp::endpoint endpoint{ asio::ip::make_address("142.250.190.78​", ec), 443 };
    asio::ip::tcp::endpoint endpoint{ asio::ip::make_address("51.38.81.49", ec), 80 };

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
        std::cout << "Failed to connect to address:\n" << ec.message() << std::endl;
    }

    if (socket.is_open())
    {
        std::string s_request
        {
            "GET / HTTP/1.1\r\n"
            "Host: example.com\r\n"
            "Connection: close\r\n\r\n"
        };

        socket.write_some(asio::buffer(s_request.data(), s_request.size()), ec);

        socket.wait(socket.wait_read);

        size_t bytes{ socket.available() };
        std::cout << "Bytes Available: " << bytes << std::endl;
        
        if (bytes > 0)
        {
            std::vector<char> v_buffer(bytes);
            socket.read_some(asio::buffer(v_buffer.data(), v_buffer.size()), ec);
            
            for (auto c : v_buffer)
            {
                std::cout << c;
            }
        }
    }

    std::cin.get();
    return 0;
}
