#pragma once

#include <cstdint>

#include "util.h"

BEGIN_UTIL_NAMESPACE

class IThreadWrapper;
class UtilTimer;

class UTIL_API UtilTimerSink
{
public:
    virtual void timer_work(const class UtilTimer* who_is) = 0;
    virtual ~UtilTimerSink() {}
};

class UTIL_API UtilTimer
{
public:
    UtilTimer(uint32_t id = 0, IThreadWrapper* own_thr = nullptr);
    virtual ~UtilTimer() {}

    virtual int32_t add_timer(const UtilTimerSink* sink,
                              uint32_t interval,
                              uint32_t times = 1);

    virtual int32_t remove_timer();

    virtual UtilTimerSink* get_sink() const { return m_sink; }

    uint32_t get_id() const { return m_id; }

    uint32_t get_times() const { return m_times; }

protected:
    const uint32_t m_id;
    uint32_t m_times;
    UtilTimerSink* m_sink;
};

END_UTIL_NAMESPACE
