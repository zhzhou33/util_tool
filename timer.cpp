#include "timer.h"
#include "thread_timer.h"

#include <cstdint>


Timer::Timer(int64_t when_ms,
             int64_t interval_ms,
             int32_t times,
             util_timer* who_is)
    : m_id(__COUNTER__)
    , m_intervalMs(interval_ms)
    , m_repeated(times > 0)
    , m_whenMs(when_ms)
    , m_times(times)
    , m_who(who_is)
{
}

void Timer::run()
{
    util_timer_sink* sink = m_who->get_sink();
    if (sink)
    {
        sink->timer_work(m_who);
    }
}

util_timer::util_timer(uint32_t id, IThreadWrapper* ownThr)
    : m_id(0 == id ? __COUNTER__ : id)
    , m_times(0)
    , m_sink(nullptr)
{
}

int32_t util_timer::add_timer(const util_timer_sink* sink,
                            uint32_t interval,
                            uint32_t times)
{
    // 获取当前线程的 ThreadTimer
    ThreadTimer* threadTimer = ThreadTimer::current();
    if (!threadTimer)
    {
        // 如果当前线程没有 ThreadTimer，返回错误
        return -1;
    }

    m_sink = const_cast<util_timer_sink*>(sink);
    threadTimer->add_timer(interval, times, this);
    return 0;
}

int32_t util_timer::remove_timer()
{
    // 获取当前线程的 ThreadTimer
    ThreadTimer* threadTimer = ThreadTimer::current();
    if (!threadTimer)
    {
        return -1;
    }

    threadTimer->remove_timer(this->m_id);
    return 0;
}

