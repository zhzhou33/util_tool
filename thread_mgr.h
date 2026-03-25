#pragma once

#include <list>
#include <mutex>

#include "util_common.h"

BEGIN_UTIL_NAMESPACE

class ThreadWrapper;
class TimerDriver;

class ThreadMgrImpl
{
public:

    ThreadWrapper* create_thread(thread_type type = IO_THREAD);
    ThreadWrapper* get_main_thread();
    ThreadWrapper* current_thread();

    TimerDriver* get_driver() { return m_driver; }

    std::vector<ThreadWrapper*>* get_thread_list() { return &m_threads; }

    std::mutex* get_mutex() { return &m_thrMgrMutex; }

    ThreadMgrImpl();
    ~ThreadMgrImpl();

private:
    std::vector<ThreadWrapper*> m_threads;
    ThreadWrapper* m_mainThread;
    TimerDriver* m_driver;
    std::mutex m_thrMgrMutex;
};

END_UTIL_NAMESPACE
