#ifndef TIMER_H_
#define TIMER_H_

#include <cstddef>
#include <cstdint>
#include <memory>

#include "util_common.h"

BEGIN_UTIL_NAMESPACE
USING_UTIL_NAMESPACE

// 前向声明
class thread_wrapper_t;

class Timer;

class Timer
{
public:
    Timer(int64_t when_ms,
          int64_t interval_ms,
          int32_t times,
          class timer_it* who_is);

    void run();

    uint32_t id() const { return m_id; }

    int64_t when_ms() const { return m_whenMs; }

    bool repeated() const { return m_repeated; }

    void update_when_time() { m_whenMs += m_intervalMs; }

    int32_t get_times() const { return m_times; }

    int32_t decrease_times()
    {
        if (m_times > 0)
            --m_times;
        return m_times;
    }

private:
    uint32_t m_id;
    timer_it* m_who;
    int64_t m_whenMs;
    uint32_t m_intervalMs;
    bool m_repeated;
    int32_t m_times;
};

using TimerPtr = std::shared_ptr<Timer>;

class timer_sink_it
{
public:
    virtual void timer_work(const class timer_it* who_is) = 0;
    virtual ~timer_sink_it() {}
};

class timer_it
{
public:
    timer_it(uint32_t id = 0, thread_wrapper_t* own_thr = nullptr);
    virtual ~timer_it() {}

    virtual int32_t add_timer(const timer_sink_it* sink,
                              uint32_t interval,
                              uint32_t times = 1);

    virtual int32_t remove_timer() { return 0; }

    virtual timer_sink_it* get_sink() const { return 0; }

    uint32_t get_id() const { return m_id; }

    uint32_t get_times() const { return m_times; }

protected:
    const uint32_t m_id;
    uint32_t m_times;
};

class timer_elem_t : public timer_it
{
public:
    timer_elem_t(uint32_t id = 0, thread_wrapper_t* own_thr = nullptr);
    virtual ~timer_elem_t() {}

    virtual int32_t add_timer(const timer_sink_it* sink,
                              uint32_t interval,
                              uint32_t times = 1);

    virtual int32_t remove_timer();

    virtual timer_sink_it* get_sink() const { return m_sink; }

protected:
    timer_sink_it* m_sink;
};

END_UTIL_NAMESPACE

#endif  // TIMER_H_

