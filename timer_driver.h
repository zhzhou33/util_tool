#ifndef TIMER_DRIVER_H_
#define TIMER_DRIVER_H_

#include <cstdint>
#include <mutex>
#include <vector>

#include "util.h"
#include "util_common.h"

USING_UTIL_NAMESPACE
BEGIN_UTIL_NAMESPACE

class ThreadWrapper;

/*
TimerDriver - 定时器驱动（主线程模式）
设计思路
- 单例模式
- 由主线程调用 run_once()驱动
- 向所有注册的 ThreadWrapper 广播 TickMsg
- 各线程收到消息后处理时间轮
*/
class TimerDriver
{
public:
    static TimerDriver *get_instance()
    {
        static TimerDriver instance;
        return &instance;
    }

    // 在当前线程执行一次驱动（主线程调用）
    void run_once();

    uint32_t get_step_ms() const { return m_steps; }

    void set_thread_list(std::vector<ThreadWrapper *> *thrs) { m_thrs = thrs; }

private:
    TimerDriver();
    ~TimerDriver();

private:
    uint32_t m_steps;
    int64_t m_preTime;
    std::vector<ThreadWrapper *> *m_thrs;
};

END_UTIL_NAMESPACE

#endif // TIMER_DRIVER_H_