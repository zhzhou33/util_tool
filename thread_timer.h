#ifndef THREAD_TIMER_H_
#define THREAD_TIMER_H_

#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>

#include "time_wheel.h"
#include "util_common.h"

BEGIN_UTIL_NAMESPACE

#define TIMESTEMPS 10

class timer_it;

class ThreadTimer
{
public:
    ThreadTimer();
    ~ThreadTimer();

    void init();

    uint32_t add_timer(uint32_t interval, uint32_t times, timer_it* who_is);
    void remove_timer(uint32_t timer_id);

    void on_tick(uint32_t elapsed_ticks);

    static ThreadTimer* current();
    static void set_current(ThreadTimer* timer);

private:
    void append_time_wheel(uint32_t scales,
                           uint32_t scaleUnitMs,
                           const std::string& name = "");
    TimeWheelPtr get_greatest_time_wheel();
    TimeWheelPtr get_least_time_wheel();

private:
    std::unordered_set<uint32_t> m_cancelTimerId;
    uint32_t m_timerStepMs;
    std::vector<TimeWheelPtr> m_timeWheels;

    static thread_local ThreadTimer* t_current;
};

END_UTIL_NAMESPACE

#endif  // THREAD_TIMER_H_
