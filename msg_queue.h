#ifndef MSG_QUEUE_H_
#define MSG_QUEUE_H_

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>
#include "util.h"

using MsgPtr = std::shared_ptr<util::ut_msg>;

/**
 * SPSC 无锁环形队列（单生产者单消费者）
 * - push: 无锁
 * - pop: 无锁
 */
template <typename T>
class SPSCQueue
{
public:
    SPSCQueue(size_t capacity = 1024)
        : _capacity(capacity)
        , _buffer(new T[capacity])
        , _head(0)
        , _tail(0)
    {
    }

    ~SPSCQueue()
    {
        delete[] _buffer;
    }

    bool push(const T& item)
    {
        size_t head = _head.load(std::memory_order_relaxed);
        size_t next = (head + 1) % _capacity;

        if (next == _tail.load(std::memory_order_acquire))
        {
            return false;  // 队列满
        }

        _buffer[head] = item;
        _head.store(next, std::memory_order_release);
        return true;
    }

    bool pop(T& item)
    {
        size_t tail = _tail.load(std::memory_order_relaxed);

        if (tail == _head.load(std::memory_order_acquire))
        {
            return false;  // 队列空
        }

        item = std::move(_buffer[tail]);
        _tail.store((tail + 1) % _capacity, std::memory_order_release);
        return true;
    }

    bool empty() const
    {
        return _tail.load(std::memory_order_acquire) ==
               _head.load(std::memory_order_acquire);
    }

private:
    size_t _capacity;
    T* _buffer;
    std::atomic<size_t> _head;
    std::atomic<size_t> _tail;
};

/**
 * MPSC 消息队列（多生产者单消费者）- 用于控制消息
 */
class MsgQueue
{
public:
    MsgQueue(size_t capacity = 1024)
        : _capacity(capacity)
    {
        _writeQueue.reserve(capacity);
        _readQueue.reserve(capacity);
    }

    bool push(MsgPtr msg)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_writeQueue.size() >= _capacity)
        {
            return false;
        }
        _writeQueue.push_back(std::move(msg));
        return true;
    }

    std::vector<MsgPtr> pop_all()
    {
        if (_readQueue.empty())
        {
            std::vector<MsgPtr> result;
            result.swap(_readQueue);
            return result;
        }

        {
            std::lock_guard<std::mutex> lock(_mutex);
            _readQueue.swap(_writeQueue);
        }

        std::vector<MsgPtr> result;
        result.swap(_readQueue);
        return result;
    }

    bool empty()
    {
        if (!_readQueue.empty())
        {
            return false;
        }
        std::lock_guard<std::mutex> lock(_mutex);
        return _writeQueue.empty();
    }

private:
    std::mutex _mutex;
    std::vector<MsgPtr> _writeQueue;
    std::vector<MsgPtr> _readQueue;
    size_t _capacity;
};

// SPSC Tick 计数器
class TickCounter
{
public:
    void add(uint32_t ticks)
    {
        _ticks.fetch_add(ticks, std::memory_order_relaxed);
    }

    uint32_t consume()
    {
        return _ticks.exchange(0, std::memory_order_acquire);
    }

    bool has_ticks() const
    {
        return _ticks.load(std::memory_order_relaxed) > 0;
    }

private:
    std::atomic<uint32_t> _ticks{0};
};

#endif  // MSG_QUEUE_H_
