#pragma once

#include <list>

#include "util_common.h"

BEGIN_UTIL_NAMESPACE

class ThreadWrapper;
class TimerDriver;

class ThreadMgr : public NoCopy
{
public:
    static ThreadMgr* get_instance()
    {
        static ThreadMgr instance;
        return &instance;
    }

    ThreadWrapper* create_thread(thread_type type = IO_THREAD);
    ThreadWrapper* get_main_thread();
    ThreadWrapper* current_thread();

    TimerDriver* get_driver() { return m_driver; }

private:
    ThreadMgr();
    ~ThreadMgr();

private:
    std::list<ThreadWrapper*> m_threads;
    ThreadWrapper* m_mainThread;
    TimerDriver* m_driver;
};

END_UTIL_NAMESPACE
