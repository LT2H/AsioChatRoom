#include <NetCommon/NetCommon.h>

#include <iostream>
#include <vector>
#include <chrono>

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif
// #define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

// std::vector<char> buffer(20 * 1024); // 20kb buffer
std::vector<char> buffer(1 * 1024); // 1kb buffer

void grab_some_data(asio::ip::tcp::socket& socket)
{
    // The instruction we primed the context with was this read_some func, so it'll
    // read as much as it can and display it to the console
    socket.async_read_some(
        asio::buffer(buffer.data(), buffer.size()), [&](std::error_code ec, std::size_t length) {
            if (!ec)
            {
                std::cout << "\n\nRead " << length << " bytes\n\n";

                for (const auto& c : buffer)
                {
                    std::cout << c;
                }

                grab_some_data(socket);
            }
        });
}

int main()
{
    asio::error_code ec;

    // Create a "context" - essentially the platform specific interface - which is
    // the space where asio can do its work
    asio::io_context context;

    // Give some fake tasks to asio so the context doesn't finish
    asio::io_context::work idle_work{context};

    // Start the context
    // Allow that context to run in its own thread, so if it does need to stop and
    // wait, it doesn't block the main program execution
    std::thread thr_context{std::thread([&] { context.run(); })};

    // Get the address of somewhere we wish to connect to
    asio::ip::tcp::endpoint endpoint{asio::ip::make_address("51.38.81.49", ec), 80};

    // Create a socket, the context will deliver the impl
    asio::ip::tcp::socket socket{context};

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
        // We prime the asio context with an instruction to read some data if it's
        // available
        grab_some_data(socket);

        // Once it's primed, we then write our HTTP request, and then we wait
        std::string request{"GET / HTTP/1.1\r\n"
                            "Host: example.com\r\n"
                            "Connection: close\r\n\r\n"};

        socket.write_some(asio::buffer(request.data(), request.size()), ec);

        // Program does something else, while asio handles data transfer in
        // background
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(2000ms);
    }

    std::cin.get();
    return 0;
}
