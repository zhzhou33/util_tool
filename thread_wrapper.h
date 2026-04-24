#pragma once

#include <atomic>
#include <cstddef>
#include <thread>
#include <unordered_map>
#include <mutex>

#include "msg_queue.h"
#include "notify.h"
#include "util.h"

USING_UTIL_NAMESPACE
BEGIN_UTIL_NAMESPACE

class ThreadTimer;
class TimerDriver;
class ThreadMgr;
class ThreadWrapper;

/**
 * Channel - 两个线程之间的通信通道（双向 SPSC 无锁）
 */
struct Channel
{
    // MPSC<MsgPtr, MSG_QUEUE_SIZE> *writeQueue; // 我写，对方读,指向对方的读取队列
    MPSC<MsgPtr, MSG_QUEUE_SIZE> *readQueue;  // 对方写，我读,只有一个队列

    Channel() : readQueue(nullptr) {}
};

/**
 * ThreadWrapper - 线程包装器（类似 tp_util 的 thread_wrapper_t）
 */
class ThreadWrapper : public IThreadWrapper
{
public:
    ThreadWrapper(ThreadMgrImpl *threadMgr, ThreadType type = IO_THREAD);
    ~ThreadWrapper();

    // 单次执行（主线程调用）
    void run_once(uint32_t ticks);

    // 发送消息到目标线程（通过 SPSC 通道，无锁）
    bool post_msg(ThreadWrapper *target, UtilMsg *data);

    // 发送消息（兼容旧接口，使用控制队列）
    void post_msg(UtilMsg *data) override;

    void thread_run() override;

    // 创建与目标线程的通道
    void create_channel(ThreadWrapper *peer, size_t capacity = 256);

    // 获取定时器
    ThreadTimer *get_timer() { return m_timer; }

    // 停止线程
    void stop();

    // 获取 Tick 计数器
    TickCounter *get_tick_counter() { return &m_tickCounter; }

    // 唤醒线程
    void wakeup();

    // 线程局部存储
    static ThreadWrapper *current() { return t_current; }
    static void set_current(ThreadWrapper *wrapper) { t_current = wrapper; }

private:
    void process_messages(bool autoTick = false, uint32_t ticks = 0);
    void thread_func();

private:
    std::thread *m_thread;
    std::atomic<bool> m_running{true};
    ThreadMgrImpl *m_threadMgr;
    ThreadType m_type;

    // 与其他线程的通道（peer -> Channel）
    std::unordered_map<ThreadWrapper *, MPSC<MsgPtr, MSG_QUEUE_SIZE>*> m_channels;
    MPSC<MsgPtr, MSG_QUEUE_SIZE>* m_readEventQueue;

    TickCounter m_tickCounter;
    ThreadTimer *m_timer;
    notify m_notify;

    static thread_local ThreadWrapper *t_current;

public:
    static std::mutex s_thrMutex;
};

END_UTIL_NAMESPACE
