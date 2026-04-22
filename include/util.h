#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(UTIL_SHARED)
#if defined(UTIL_BUILD_DLL)
#define UTIL_API __declspec(dllexport)
#else
#define UTIL_API __declspec(dllimport)
#endif
#else
#define UTIL_API
#endif
#elif defined(__GNUC__) && __GNUC__ >= 4
#define UTIL_API __attribute__((visibility("default")))
#else
#define UTIL_API
#endif

#define BEGIN_UTIL_NAMESPACE \
    namespace util           \
    {

#define END_UTIL_NAMESPACE }

#define USING_UTIL_NAMESPACE using namespace util;

BEGIN_UTIL_NAMESPACE

enum thread_type
{
    MAIN_THREAD = 0,
    IO_THREAD,
};

class ThreadMgrImpl;

class UTIL_API util_msg
{
public:
    virtual ~util_msg() = default;
    virtual void on_message() = 0;
};

class UTIL_API IThreadWrapper
{
public:
    virtual void post_msg(util_msg *data) = 0;

    virtual void thread_run() = 0;
};

class UTIL_API ThreadMgr
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