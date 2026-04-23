#include "thread_mgr.h"
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

ThreadMgrImpl::ThreadMgrImpl()
{
    m_driver = TimerDriver::get_instance();
    m_driver->set_thread_list(&m_threads);
    // 创建主线程包装器（不启动新线程）
    m_mainThread = new ThreadWrapper(this, MAIN_THREAD);
    
}

ThreadMgrImpl::~ThreadMgrImpl()
{
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
