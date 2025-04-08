#pragma once

#include "NetCommon.h"

namespace olc
{
namespace net
{
template <typename T> class TsQueue
{
  public:
    TsQueue()                       = default;
    TsQueue(const TsQueue<T>& that) = delete;
    virtual ~TsQueue() { clear(); }

    // Returns and maintains item at front of Queue
    const T& front()
    {
        std::scoped_lock lock{ mux_queue_ };
        return queue_.front();
    }

    // Returns and maintains item at back of Queue
    const T& back()
    {
        std::scoped_lock lock{ mux_queue_ };
        return queue_.back();
    }

    // Adds an item to front of Queue
    void push_front(const T& item)
    {
        std::scoped_lock lock{ mux_queue_ };
        queue.emplace_front(std::move(item));
    }

    // Adds an item to back of Queue
    void push_back(const T& item)
    {
        std::scoped_lock lock{ mux_queue_ };
        queue.emplace_back(std::move(item));
    }

    // Returns number of items in Queue
    size_t count()
    {
        std::scoped_lock lock{ mux_queue_ };
        return queue_.size();
    }

    // Clear Queue
    void clear()
    {
        std::scoped_lock lock{ mux_queue_ };
        queue_.clear();
    }

    // Removes and returns item from front of Queue
    T pop_front()
    {
        std::scoped_lock lock{ mux_queue_ };
        auto t{ std::move(queue.front()) };
        queue.pop_front();
        return t;
    }

    // Removes and returns item from back of Queue
    T pop_back()
    {
        std::scoped_lock lock{ mux_queue_ };
        auto t{ std::move(queue.front()) };
        queue.pop_back();
        return t;
    }

    template <typename T> class Connection;

    template <typename T> struct OwnedMessage
    {
        std::shared_ptr<Connection<T>> remote{ nullptr };
        Message<T> msg;

        // Again, a friendly string maker
        friend std::ostream& operator<<(std::ostream& os, const OwnedMessage<T>& msg)
        {
            os << msg.msg;
            return os;
        }
    };


  protected:
    std::mutex mux_queue_;
    std::deque<T> queue_;
};
} // namespace net
} // namespace olc