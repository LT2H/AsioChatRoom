#pragma once

#include <iostream>
#include <memory>
#include <thread>
#include <mutex>
#include <deque>
#include <optional>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <array>
#include <unordered_map>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <system_error>

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <asio/async_result.hpp>
#include <asio/connect.hpp>
#include <asio/impl/read.hpp>
#include <asio/impl/write.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/post.hpp>