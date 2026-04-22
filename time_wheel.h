#ifndef TIME_WHEEL_H_
#define TIME_WHEEL_H_

#include <chrono>
#include <cstdint>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include "timer.h"
#include "util_common.h"

USING_UTIL_NAMESPACE
BEGIN_UTIL_NAMESPACE


class TimeWheel
{
public:
    TimeWheel(uint32_t scales, uint32_t scale_unit_ms,
              const std::string& name = "");

    uint32_t scale_unit_ms() const { return m_scaleUnitMs; }

    uint32_t scales() const { return m_scales; }

    uint32_t current_index() const { return m_curIndex; }

    void set_less_level_tw(TimeWheel* lessLevelTW)
    {
        m_pLessLevelTW = lessLevelTW;
    }

    void set_greater_level_tw(TimeWheel* greaterLevelTW)
    {
        m_pGreaterLevelTW = greaterLevelTW;
    }

    int64_t get_current_time() const;

    void add_timer(TimerPtr timer);

    void increase();

    std::list<TimerPtr> get_and_clear_current_slot();

private:
    std::string m_strName;
    uint32_t m_curIndex;

    // A time wheel can be divided into multiple scales. A scale has N ms.
    uint32_t m_scales;
    uint32_t m_scaleUnitMs;

    // Every slot corresponds to a scale. Every slot contains the timers.
    std::vector<std::list<TimerPtr>> m_slots;

    TimeWheel* m_pLessLevelTW;     // Less scale unit.
    TimeWheel* m_pGreaterLevelTW;  // Greater scale unit.
};

using TimeWheelPtr = std::shared_ptr<TimeWheel>;

inline int64_t get_now_time_stamp()
{
    using namespace std::chrono;
    auto now = system_clock::now().time_since_epoch();
    return duration_cast<milliseconds>(now).count();
}

inline int64_t get_steady_clock_time()
{
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

END_UTIL_NAMESPACE

#endif  // TIME_WHEEL_H_