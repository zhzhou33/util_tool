#ifndef TIMER_DRIVER_H_
#define TIMER_DRIVER_H_

#include <cstdint>
#include <mutex>
#include <vector>

#include "util.h"
#include "util_common.h"

USING_UTIL_NAMESPACE
BEGIN_UTIL_NAMESPACE

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
#define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002
#endif



// 高精度等待定时器（需要 Windows 10 1803+ 或 Windows 11）
class HighPrecisionWaitableTimer
{
public:
    HighPrecisionWaitableTimer()
    {
        // 尝试创建高精度定时器
        m_timer = CreateWaitableTimerExW(
            NULL,                                  // 安全属性
            NULL,                                  // 名称
            CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, // 关键！高精度标志
            TIMER_ALL_ACCESS                       // 访问权限
        );

        if (!m_timer)
        {
            // 降级到普通定时器
            m_timer = CreateWaitableTimer(NULL, TRUE, NULL);
            m_highResolution = false;
        }
        else
        {
            m_highResolution = true;
        }
    }

    ~HighPrecisionWaitableTimer()
    {
        if (m_timer) CloseHandle(m_timer);
    }

    // 等待指定的毫秒数
    bool wait_for(int64_t milliseconds)
    {
        if (milliseconds <= 0) return true;

        LARGE_INTEGER dueTime;
        // 负数表示相对时间，单位是 100 纳秒
        // 10ms = 10 * 1000 * 100 纳秒 = 1,000,000 个 100ns 单位
        dueTime.QuadPart = -static_cast<LONGLONG>(milliseconds) * 10000;

        // 设置定时器
        if (!SetWaitableTimer(m_timer, &dueTime, 0, NULL, NULL, FALSE))
        {
            return false;
        }

        // 等待定时器触发
        DWORD result = WaitForSingleObject(m_timer, INFINITE);
        return result == WAIT_OBJECT_0;
    }

    bool isHighResolution() const { return m_highResolution; }

private:
    HANDLE m_timer;
    bool m_highResolution;
};
#endif

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

    void precise_sleep(int64_t ms);

private:
    TimerDriver();
    ~TimerDriver();

private:
    uint32_t m_steps;
    int64_t m_preTime;
    std::vector<ThreadWrapper *> *m_thrs;
    bool m_highResAvailable;
#ifdef WIN32
    HighPrecisionWaitableTimer m_timer;
#endif
};

END_UTIL_NAMESPACE

#endif // TIMER_DRIVER_H_