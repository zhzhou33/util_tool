#include "thread_wrapper.h"

#include <utility>

#include "thread_mgr.h"
#include "thread_timer.h"
#include "timer_driver.h"

USING_UTIL_NAMESPACE

thread_local ThreadWrapper *ThreadWrapper::t_current = nullptr;

std::mutex ThreadWrapper::s_thrMutex;

ThreadWrapper::ThreadWrapper(ThreadMgrImpl *threadMgr, thread_type type) :
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
    auto thrList = m_threadMgr->get_thread_list();
    std::lock_guard<std::mutex> lock(s_thrMutex);
    for (const auto &thr : *thrList)
    {
        thr->create_channel(this);
    }
    thrList->push_back(this);

    // // 注册到 TimerDriver
    // this->register_to_driver();
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

    for (auto &pair : m_channels)
    {
        delete pair.second.writeQueue;
    }
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

void ThreadWrapper::run_once()
{
    process_messages(true);
}

void ThreadWrapper::process_messages(bool autoTick)
{
    if (autoTick)
    {
        m_timer->on_tick(1);
    }
    else
    {
        m_timer->on_tick(m_tickCounter.consume());
    }

    for (auto &pair : m_channels)
    {
        Channel &ch = pair.second;
        if (ch.readQueue)
        {
            MsgPtr msg;
            while (ch.readQueue->pop(msg))
            {
                msg->on_message();
            }
        }
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

    auto *queue1 = new SPSCQueue<MsgPtr>(capacity); // this->peer
    auto *queue2 = new SPSCQueue<MsgPtr>(capacity); // peer->this

    Channel &myChannel = m_channels[peer];
    myChannel.writeQueue = queue1;
    myChannel.readQueue = queue2;

    Channel &peerChannel = peer->m_channels[this];
    peerChannel.writeQueue = queue2;
    peerChannel.readQueue = queue1;
}

bool ThreadWrapper::post_msg(ThreadWrapper *target, ut_msg *data)
{
    if (target == nullptr || data == nullptr)
    {
        return false;
    }

    auto it = m_channels.find(target);
    if (it != m_channels.end() && it->second.writeQueue != nullptr && target)
    {
        Channel ch = it->second;
        bool ok = it->second.writeQueue->push(std::shared_ptr<ut_msg>(data));
        if (ok)
        {
            target->wakeup();
        }
        return ok;
    }
    return false;
}

void ThreadWrapper::post_msg(ut_msg *data)
{
    ThreadWrapper *sender = ThreadWrapper::current();
    if (sender != nullptr && sender != this)
    {
        sender->post_msg(this, data);
        return;
    }
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
    m_threadMgr->get_driver()->run_once();
    this->run_once();
}
