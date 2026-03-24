#include "thread_mgr.h"
#include "thread_wrapper.h"
#include "timer_driver.h"

ThreadMgr::ThreadMgr()
{
    m_driver = TimerDriver::get_instance();

    // 创建主线程包装器（不启动新线程）
    m_mainThread = new ThreadWrapper(this, MAIN_THREAD);
    m_threads.push_back(m_mainThread);

    // 注册到 TimerDriver
    m_mainThread->register_to_driver();
}

ThreadMgr::~ThreadMgr()
{
    for (auto* thr : m_threads)
    {
        delete thr;
    }
    m_threads.clear();
}

ThreadWrapper* ThreadMgr::create_thread(thread_type type)
{
    ThreadWrapper* wrapper = new ThreadWrapper(this, type);
    m_threads.push_back(wrapper);

    // 注册到 TimerDriver
    wrapper->register_to_driver();

    return wrapper;
}

ThreadWrapper* ThreadMgr::get_main_thread()
{
    // if (!m_mainThread)
    // {
    //     // 创建主线程包装器（不启动新线程）
    //     m_mainThread = new ThreadWrapper(this, MAIN_THREAD);
    //     m_threads.push_back(m_mainThread);
    //
    //     // 注册到 TimerDriver
    //     m_mainThread->register_to_driver(m_driver);
    // }
    return m_mainThread;
}

ThreadWrapper* ThreadMgr::current_thread()
{
    return ThreadWrapper::current();
}

