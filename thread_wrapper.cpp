#include "thread_wrapper.h"

#include <utility>

#include "msg_queue.h"
#include "thread_mgr.h"
#include "thread_timer.h"
#include "time_wheel.h"
#include "timer_driver.h"

thread_local ThreadWrapper *ThreadWrapper::t_current = nullptr;

std::mutex ThreadWrapper::s_thrMutex;

ThreadWrapper::ThreadWrapper(ThreadMgrImpl *threadMgr, ThreadType type) :
    m_thread(nullptr),
    m_threadMgr(threadMgr),
    m_type(type),
    m_timer(new ThreadTimer()),
    m_notify(TIMESTEMPS)
{
    m_timer->init();

    if (type == IO_THREAD)
    {
        m_thread = new std::thread(&ThreadWrapper::thread_func, this);
    }
    else
    {
        ThreadWrapper::set_current(this);
        ThreadTimer::set_current(m_timer);
    }
    // init read channel queue
    m_readEventQueue = new MPSC<MsgPtr, MSG_QUEUE_SIZE>();
    auto thrList = m_threadMgr->get_thread_list();
    std::lock_guard<std::mutex> lock(s_thrMutex);
    for (const auto &thr : *thrList)
    {
        thr->m_channels[this] = m_readEventQueue;
        this->m_channels[thr] = thr->m_readEventQueue;
    }
    thrList->push_back(this);
}

ThreadWrapper::~ThreadWrapper()
{
    stop();
    if (m_thread != nullptr)
    {
        if (m_thread->joinable())
        {
            m_thread->join();
        }
        delete m_thread;
        m_thread = nullptr;
    }

    delete m_readEventQueue;
    m_channels.clear();

    delete m_timer;
    m_timer = nullptr;
}

void ThreadWrapper::stop()
{
    m_running = false;
    m_notify.signal();
}

void ThreadWrapper::thread_func()
{
    ThreadWrapper::set_current(this);
    ThreadTimer::set_current(m_timer);

    while (m_running)
    {
        m_notify.wait();
        if (!m_running)
        {
            break;
        }
        process_messages();
    }
}

void ThreadWrapper::run_once(uint32_t ticks)
{
    process_messages(true, ticks);
}

void ThreadWrapper::process_messages(bool autoTick, uint32_t ticks)
{
    if (autoTick)
    {
        m_timer->on_tick(ticks);
    }
    else
    {
        m_timer->on_tick(m_tickCounter.consume());
    }

    size_t events = m_readEventQueue->available();
    if (events != 0)
        std::cout << "handle evnets " << events << std::endl;
    MsgPtr msg;
    for (int i = 0; i < events; i++)
    {
        m_readEventQueue->pop(msg);
        msg->on_message();
    }
}

void ThreadWrapper::create_channel(ThreadWrapper *peer, size_t capacity)
{
    if (peer == nullptr || peer == this)
    {
        return;
    }

    if (m_channels.find(peer) != m_channels.end())
    {
        return;
    }
}

bool ThreadWrapper::post_msg(ThreadWrapper *target, UtilMsg *data)
{
    // if (target == nullptr || data == nullptr)
    // {
    //     return false;
    // }

    // auto it = m_channels.find(target);
    // if (it != m_channels.end() && target->m_readEventQueue != nullptr)
    // {
    //     Channel ch = it->second;
    //     bool ok = it->second.writeQueue->push(std::shared_ptr<UtilMsg>(data));
    //     if (ok)
    //     {
    //         target->wakeup();
    //     }
    //     return ok;
    // }
    return false;
}

void ThreadWrapper::post_msg(UtilMsg *data)
{
    if (this->m_readEventQueue->push(std::shared_ptr<UtilMsg>(data)))
    {
        this->wakeup();
    }

    // ThreadWrapper *sender = ThreadWrapper::current();
    // if (sender != nullptr && sender != this)
    // {
    //     sender->post_msg(this, data);
    //     return;
    // }
}

void ThreadWrapper::wakeup()
{
    m_notify.signal();
}

// void ThreadWrapper::register_to_driver()
// {
//     m_threadMgr->get_driver()->register_wrapper(this);
// }

void ThreadWrapper::thread_run()
{
    uint32_t ticks = m_threadMgr->get_driver()->run_once();
    this->run_once(ticks);
}
