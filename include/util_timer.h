#pragma once

#include <cstdint>

#include "util.h"

BEGIN_UTIL_NAMESPACE

class IThreadWrapper;
class util_timer;

class UTIL_API util_timer_sink
{
public:
    virtual void timer_work(const class util_timer* who_is) = 0;
    virtual ~util_timer_sink() {}
};

class UTIL_API util_timer
{
public:
    util_timer(uint32_t id = 0, IThreadWrapper* own_thr = nullptr);
    virtual ~util_timer() {}

    virtual int32_t add_timer(const util_timer_sink* sink,
                              uint32_t interval,
                              uint32_t times = 1);

    virtual int32_t remove_timer();

    virtual util_timer_sink* get_sink() const { return m_sink; }

    uint32_t get_id() const { return m_id; }

    uint32_t get_times() const { return m_times; }

protected:
    const uint32_t m_id;
    uint32_t m_times;
    util_timer_sink* m_sink;
};

END_UTIL_NAMESPACE
