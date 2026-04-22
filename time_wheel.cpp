#include "time_wheel.h"

#include <utility>


TimeWheel::TimeWheel(uint32_t scales, uint32_t scale_unit_ms,
                     const std::string& name)
    : m_strName(name)
    , m_curIndex(0)
    , m_scales(scales)
    , m_scaleUnitMs(scale_unit_ms)
    , m_slots(scales)
    , m_pLessLevelTW(nullptr)
    , m_pGreaterLevelTW(nullptr)
{
}

int64_t TimeWheel::get_current_time() const
{
    int64_t t = static_cast<int64_t>(m_curIndex) * m_scaleUnitMs;
    if (m_pLessLevelTW != nullptr)
    {
        t += m_pLessLevelTW->get_current_time();
    }
    return t;
}

void TimeWheel::add_timer(TimerPtr timer)
{
    int64_t less_tv_time = 0;
    if (m_pLessLevelTW != nullptr)
    {
        // Elapsed time inside the current slot of the smaller (less) wheel.
        less_tv_time = m_pLessLevelTW->get_current_time();
    }

    // Align this wheel's slot calculation using the smaller wheel's offset.
    int64_t diff = timer->when_ms() + less_tv_time - get_now_time_stamp();
    if (diff >= m_scaleUnitMs)
    {
        size_t n = (m_curIndex + (diff + 5) / m_scaleUnitMs) % m_scales;
        m_slots[n].push_back(timer);
        return;
    }

    if (m_pLessLevelTW != nullptr)
    {
        m_pLessLevelTW->add_timer(timer);
        return;
    }

    m_slots[m_curIndex].push_back(timer);
}

void TimeWheel::increase()
{
    ++m_curIndex;
    if (m_curIndex < m_scales)
    {
        return;
    }

    m_curIndex = 0;
    if (m_pGreaterLevelTW != nullptr)
    {
        m_pGreaterLevelTW->increase();
        std::list<TimerPtr> slot =
            std::move(m_pGreaterLevelTW->get_and_clear_current_slot());
        for (const TimerPtr& timer : slot)
        {
            add_timer(timer);
        }
    }
}

std::list<TimerPtr> TimeWheel::get_and_clear_current_slot()
{
    std::list<TimerPtr> slot;
    slot = std::move(m_slots[m_curIndex]);
    return slot;
}
