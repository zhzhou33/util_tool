#pragma once

#include <cstdint>
#include <cstddef>
#include <netdb.h>
#include "util_common.h"

BEGIN_UTIL_NAMESPACE

class ThreadMgrImpl;

class ut_msg
{
public:
    virtual ~ut_msg() = default;
    virtual void on_message() = 0;
};

class IThreadWrapper
{
public:
    virtual void post_msg(ut_msg *data) = 0;

    virtual void thread_run() = 0;
};

class ThreadMgr : public NoCopy
{
public:
    static ThreadMgr *get_instance()
    {
        static ThreadMgr instance;
        return &instance;
    }

    IThreadWrapper *create_thread(thread_type type = IO_THREAD);
    IThreadWrapper *get_main_thread();
    IThreadWrapper *current_thread();

private:
    ThreadMgr();
    ~ThreadMgr();

private:
    ThreadMgrImpl *m_impl;
};

END_UTIL_NAMESPACE
