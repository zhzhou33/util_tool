#include "thread_timer.h"
#include "time_wheel.h"


// 线程局部存储
thread_local ThreadTimer *ThreadTimer::t_current = nullptr;

ThreadTimer::ThreadTimer() : m_timerStepMs(TIMESTEMPS)
{
}

ThreadTimer::~ThreadTimer() = default;

void ThreadTimer::init()
{
    // 创建多层时间轮
    append_time_wheel((1u << 8), (1u << 8) * (1u << 8) * 10, "Level_2");
    append_time_wheel((1u << 8), (1u << 8) * 10, "Level_1");
    append_time_wheel(1u << 8, 10, "Level_0");
}

void ThreadTimer::append_time_wheel(uint32_t scales,
                                    uint32_t scaleUnitMs,
                                    const std::string &name)
{
    TimeWheelPtr timeWheel =
        std::make_shared<TimeWheel>(scales, scaleUnitMs, name);

    if (m_timeWheels.empty())
    {
        m_timeWheels.push_back(timeWheel);
        return;
    }

    TimeWheelPtr greaterTimeWheel = m_timeWheels.back();
    greaterTimeWheel->set_less_level_tw(timeWheel.get());
    timeWheel->set_greater_level_tw(greaterTimeWheel.get());
    m_timeWheels.push_back(timeWheel);
}

TimeWheelPtr ThreadTimer::get_greatest_time_wheel()
{
    if (m_timeWheels.empty())
    {
        return TimeWheelPtr();
    }
    return m_timeWheels.front();
}

TimeWheelPtr ThreadTimer::get_least_time_wheel()
{
    if (m_timeWheels.empty())
    {
        return TimeWheelPtr();
    }
    return m_timeWheels.back();
}

uint32_t ThreadTimer::add_timer(uint32_t interval,
                                uint32_t times,
                                util_timer *who_is)
{
    if (m_timeWheels.empty())
    {
        return static_cast<uint32_t>(-1);
    }

    int64_t when = get_now_time_stamp() + interval;
    get_greatest_time_wheel()->add_timer(
        std::make_shared<Timer>(when, interval, times, who_is));
    return 0;
}

void ThreadTimer::remove_timer(uint32_t timer_id)
{
    m_cancelTimerId.insert(timer_id);
}

void ThreadTimer::on_tick(uint32_t elapsed_ticks)
{
    if (m_timeWheels.empty())
    {
        return;
    }

    for (uint32_t i = 0; i < elapsed_ticks; ++i)
    {
        TimeWheelPtr leastTimeWheel = get_least_time_wheel();
        leastTimeWheel->increase();
        std::list<TimerPtr> slot =
            std::move(leastTimeWheel->get_and_clear_current_slot());

        for (const TimerPtr &timer : slot)
        {
            auto it = m_cancelTimerId.find(timer->id());
            if (it != m_cancelTimerId.end())
            {
                m_cancelTimerId.erase(it);
                continue;
            }

            timer->run();

            if (timer->decrease_times() != 0)
            {
                timer->update_when_time();
                get_greatest_time_wheel()->add_timer(timer);
            }
        }
    }
}

ThreadTimer *ThreadTimer::current()
{
    return t_current;
}

void ThreadTimer::set_current(ThreadTimer *timer)
{
    t_current = timer;
}
