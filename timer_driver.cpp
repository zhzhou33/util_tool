#include "timer_driver.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <thread>

#include "thread_wrapper.h"
#include "thread_timer.h"
#include "time_wheel.h"
#include "util_common.h"

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

uint32_t TimerDriver::run_once()
{
    if (m_preTime == 0)
    {
        m_preTime = get_now_time_stamp();
    }

    int64_t next_time = m_preTime + m_steps;
    int64_t cur_time = get_now_time_stamp();
    int64_t sleep_time = next_time - cur_time;

    int elapsed_ticks = 1;

    if (sleep_time > 0)
    {
        // 超前，等待到预期时间
        precise_sleep(sleep_time);
        elapsed_ticks = 1;
        m_preTime = next_time; // 时间基准前进1步
    }
    else
    {
        // 落后，计算能追多少步但不超过当前时间
        int64_t behind_time = -sleep_time;     // 落后的毫秒数
        elapsed_ticks = behind_time / m_steps; // 最多能追的步数（向下取整）

        if (elapsed_ticks >= 1)
        {
            // 关键：时间基准只前进实际追的步数，不会超过cur_time
            m_preTime = m_preTime + m_steps * elapsed_ticks;

            UTIL_LOG_INFO("Catching up: behind=" << behind_time
                                                 << "ms, steps=" << m_steps
                                                 << "ms, ticks=" << elapsed_ticks
                                                 << ", new_preTime=" << m_preTime);
        }
        else
        {
            // 落后不到1步，不追赶，直接执行当前步
            elapsed_ticks = 1;
            m_preTime = next_time; // 仍然按预期前进
        }
    }
    std::lock_guard<std::mutex> lock(ThreadWrapper::s_thrMutex);
    for (int i = 1; i < (*m_thrs).size(); i++)
    {
        (*m_thrs)[i]->get_tick_counter()->add(elapsed_ticks);
        (*m_thrs)[i]->wakeup();
    }

    return elapsed_ticks;
}
