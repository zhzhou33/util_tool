#include "timer_driver.h"

#include <algorithm>
#include <chrono>
#include <thread>

#include "thread_wrapper.h"
#include "thread_timer.h"
#include "time_wheel.h"

TimerDriver::TimerDriver() : m_steps(TIMESTEMPS), m_preTime(0)
{
#ifdef _WIN32
    // Windows: 提高系统时钟精度到 1ms
    if (timeBeginPeriod(1) == TIMERR_NOERROR)
    {
        m_highResAvailable = true;
        std::cout << "Timer precision increased to 1ms" << std::endl;
    }
    else
    {
        std::cout << "Failed to increase timer precision" << std::endl;
    }
#else
    m_highResAvailable = true; // Linux/Mac 天生高精度
#endif
}

TimerDriver::~TimerDriver() = default;

void TimerDriver::precise_sleep(int64_t ms)
{
#ifdef WIN32
    m_timer.wait_for(ms);
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#endif
}

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
        precise_sleep(sleep_time);
    }
    
    m_preTime = next_time;
    std::lock_guard<std::mutex> lock(ThreadWrapper::s_thrMutex);
    for (int i = 1; i < (*m_thrs).size(); i++)
    {
        (*m_thrs)[i]->get_tick_counter()->add(1);
        (*m_thrs)[i]->wakeup();
    }
}
