#include "thread_mgr.h"

#include "thread_timer.h"
#include "thread_wrapper.h"
#include "timer_driver.h"

ThreadMgr::ThreadMgr()
{
    m_impl = new ThreadMgrImpl();
}

ThreadMgr::~ThreadMgr()
{
    delete m_impl;
}

IThreadWrapper *ThreadMgr::create_thread(ThreadType type)
{
    return m_impl->create_thread(type);
}

IThreadWrapper *ThreadMgr::get_main_thread()
{
    return m_impl->get_main_thread();
}

IThreadWrapper *ThreadMgr::current_thread()
{
    return m_impl->current_thread();
}

void ThreadMgr::run_once()
{
    m_impl->run_once();
}

bool ThreadMgr::start()
{
    return m_impl->start();
}

void ThreadMgr::stop()
{
    m_impl->stop();
}

ThreadMgrImpl::ThreadMgrImpl() : m_driverThread(nullptr), m_running(false)
{
    m_driver = TimerDriver::get_instance();
    m_driver->set_thread_list(&m_threads);
    // 创建主线程包装器（不启动新线程）
    m_mainThread = new ThreadWrapper(this, MAIN_THREAD);
    
}

ThreadMgrImpl::~ThreadMgrImpl()
{
    stop();
    for (auto thr : m_threads)
    {
        delete thr;
    }
    m_threads.clear();
}

ThreadWrapper *ThreadMgrImpl::create_thread(ThreadType type)
{
    return new ThreadWrapper(this, type);
}

ThreadWrapper *ThreadMgrImpl::get_main_thread()
{
    return m_mainThread;
}

ThreadWrapper *ThreadMgrImpl::current_thread()
{
    return ThreadWrapper::current();
}

void ThreadMgrImpl::run_once()
{
    if (m_mainThread != nullptr)
    {
        m_mainThread->thread_run();
    }
}

bool ThreadMgrImpl::start()
{
    bool expected = false;
    if (!m_running.compare_exchange_strong(expected, true))
    {
        return false;
    }

    m_driverThread = new std::thread(&ThreadMgrImpl::driver_loop, this);
    return true;
}

void ThreadMgrImpl::stop()
{
    bool expected = true;
    if (!m_running.compare_exchange_strong(expected, false))
    {
        return;
    }

    for (auto thr : m_threads)
    {
        if (thr != nullptr)
        {
            thr->stop();
        }
    }

    if (m_driverThread != nullptr)
    {
        if (m_driverThread->joinable())
        {
            m_driverThread->join();
        }
        delete m_driverThread;
        m_driverThread = nullptr;
    }
}

void ThreadMgrImpl::driver_loop()
{
    ThreadWrapper::set_current(m_mainThread);
    if (m_mainThread != nullptr)
    {
        ThreadTimer::set_current(m_mainThread->get_timer());
    }

    while (m_running.load())
    {
        run_once();
    }
}
