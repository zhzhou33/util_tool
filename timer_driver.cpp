#include "timer_driver.h"

#include <algorithm>
#include <chrono>
#include <thread>

#include "thread_wrapper.h"
#include "thread_timer.h"
#include "time_wheel.h"

TimerDriver::TimerDriver() : m_steps(TIMESTEMPS), m_preTime(0)
{
}

TimerDriver::~TimerDriver() = default;

void TimerDriver::run_once()
{
    if (m_preTime == 0)
    {
        m_preTime = get_now_time_stamp();
    }

    int64_t next_time = m_preTime + m_steps;
    int64_t cur_time = get_now_time_stamp();
    int64_t sleep_time = next_time - cur_time;

    if (sleep_time > 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
    }

    m_preTime = next_time;

    
    std::lock_guard<std::mutex> lock(ThreadWrapper::s_thrMutex);
    for (int i = 1; i < (*m_thrs).size(); i++)
    {
        (*m_thrs)[i]->get_tick_counter()->add(1);
        (*m_thrs)[i]->wakeup();
    }
}
