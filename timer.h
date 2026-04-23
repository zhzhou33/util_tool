#ifndef TIMER_H_
#define TIMER_H_

#include <cstdint>
#include <memory>

#include "util_timer.h"
#include "util.h"

USING_UTIL_NAMESPACE
BEGIN_UTIL_NAMESPACE

class Timer;
class UtilTimer;

class Timer
{
public:
    Timer(int64_t when_ms,
          int64_t interval_ms,
          int32_t times,
          class UtilTimer* who_is);

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
    UtilTimer* m_who;
    int64_t m_whenMs;
    uint32_t m_intervalMs;
    bool m_repeated;
    int32_t m_times;
};

using TimerPtr = std::shared_ptr<Timer>;

END_UTIL_NAMESPACE

#endif  // TIMER_H_

