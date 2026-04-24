#pragma once

#include <atomic>
#include <thread>
#include <vector>
#include "util.h"

BEGIN_UTIL_NAMESPACE

class ThreadWrapper;
class TimerDriver;

class ThreadMgrImpl
{
public:
    ThreadWrapper *create_thread(ThreadType type = IO_THREAD);
    ThreadWrapper *get_main_thread();
    ThreadWrapper *current_thread();
    void run_once();
    bool start();
    void stop();

    TimerDriver *get_driver() { return m_driver; }

    std::vector<ThreadWrapper *> *get_thread_list() { return &m_threads; }

    ThreadMgrImpl();
    ~ThreadMgrImpl();

private:
    void driver_loop();

private:
    std::vector<ThreadWrapper *> m_threads;
    ThreadWrapper *m_mainThread;
    TimerDriver *m_driver;
    std::thread *m_driverThread;
    std::atomic<bool> m_running;
};

END_UTIL_NAMESPACE
