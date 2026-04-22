#ifndef MSG_QUEUE_H_
#define MSG_QUEUE_H_

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include "util.h"

using MsgPtr = std::shared_ptr<util::util_msg>;

/**
 * SPSC 无锁环形队列（单生产者单消费者）
 * - push: 无锁
 * - pop: 无锁
 */
template <typename T>
class SPSCQueue
{
public:
    SPSCQueue(size_t capacity = 1024) : _capacity(capacity), _buffer(new T[capacity]), _head(0), _tail(0)
    {
    }

    ~SPSCQueue()
    {
        delete[] _buffer;
    }

    bool push(const T &item)
    {
        size_t head = _head.load(std::memory_order_relaxed);
        size_t next = (head + 1) % _capacity;

        if (next == _tail.load(std::memory_order_acquire))
        {
            return false; // 队列满
        }

        _buffer[head] = item;
        _head.store(next, std::memory_order_release);
        return true;
    }

    bool pop(T &item)
    {
        size_t tail = _tail.load(std::memory_order_relaxed);

        if (tail == _head.load(std::memory_order_acquire))
        {
            return false; // 队列空
        }

        item = std::move(_buffer[tail]);
        _tail.store((tail + 1) % _capacity, std::memory_order_release);
        return true;
    }

    bool empty() const
    {
        return _tail.load(std::memory_order_acquire) == _head.load(std::memory_order_acquire);
    }

private:
    size_t _capacity;
    T *_buffer;
    std::atomic<size_t> _head;
    std::atomic<size_t> _tail;
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

// MPSC 无锁多生产者单消费者
template <typename T, size_t N>
class MPSC
{
    static_assert((N & (N - 1)) == 0);
    alignas(64) T slots_[N];
    alignas(64) std::atomic<size_t> published_[N];
    alignas(64) std::atomic<size_t> head_{0};
    alignas(64) std::atomic<size_t> tail_{0};
    alignas(64) std::atomic<size_t> available_{0};

public:
    bool try_push_single_producer(const T &val)
    {
        auto h = head_.load(std::memory_order_relaxed);
        if (h - tail_.load(std::memory_order_acquire) >= N)
        {
            return false;
        }
        slots_[h & (N - 1)] = val;
        head_.store(h + 1, std::memory_order_release);
        return true;
    }
    bool try_pop_single_consumer(T &val)
    {
        auto t = tail_.load(std::memory_order_relaxed);
        if (t == head_.load(std::memory_order_acquire))
            return false;
        val = slots_[t & (N - 1)];
        tail_.store(t + 1, std::memory_order_release);
        return true;
    }
    bool try_push(const T &val)
    {
        auto h = head_.load(std::memory_order_relaxed);
        do
        {
            if (h - tail_.load(std::memory_order_acquire) >= N)
            {
                return false;
            }
        } while (!head_.compare_exchange_weak(h, h + 1, std::memory_order_acq_rel));
        slots_[h & (N - 1)] = val;
        published_[h & (N - 1)].store(h / N, std::memory_order_release);
        available_.fetch_add(1, std::memory_order_release);
        return true;
    }

    size_t available() const
    {
        return available_.load(std::memory_order_acquire);
    }

    bool try_pop(T &val)
    {
        auto t = tail_.load(std::memory_order_relaxed);
        // 多消费者也要先CAS抢占读序列号，之后判断是否发布
        if (published_[t & (N - 1)].load(std::memory_order_acquire) != t / N)
            return false; // 还没发布
        val = slots_[t & (N - 1)];
        tail_.store(t + 1, std::memory_order_release);
        return true;
    }
    bool pop(T &val)
    {
        auto t = tail_.load(std::memory_order_relaxed);
        while (published_[t & (N - 1)].load(std::memory_order_acquire) != t / N)
        {
            // 可选：如果队列空了就返回 false
            // if (t >= head_.load(std::memory_order_acquire))
            // {
            //     return false; // 理论上 available>0 时不会发生
            // }
            std::this_thread::yield(); // 还没发布 每次让出 10-50 μs
        }
        val = slots_[t & (N - 1)];
        tail_.store(t + 1, std::memory_order_release);
        available_.fetch_sub(1, std::memory_order_release);
        return true;
    }
};

#endif // MSG_QUEUE_H_
