#include "thread_wrapper.h"

#include <utility>

#include "thread_mgr.h"
#include "thread_timer.h"
#include "timer_driver.h"

USING_UTIL_NAMESPACE

thread_local ThreadWrapper *ThreadWrapper::t_current = nullptr;

ThreadWrapper::ThreadWrapper(ThreadMgr *threadMgr, thread_type type) :
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

    std::vector<MsgPtr> ctrlMsgs = m_ctrlQueue.pop_all();
    for (auto &msg : ctrlMsgs)
    {
        msg->on_message();
    }
}

void ThreadWrapper::create_channel(ThreadWrapper *peer, size_t capacity)
{
    if (peer == nullptr || peer == this)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(m_channelMutex);
    std::lock_guard<std::mutex> peerLock(peer->m_channelMutex);

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

bool ThreadWrapper::post_msg(ThreadWrapper *target, msg *data)
{
    if (target == nullptr || data == nullptr)
    {
        return false;
    }

    class WrappingMsg : public msg_it
    {
    public:
        explicit WrappingMsg(msg *raw) : m_raw(raw)
        {
        }

        void on_message() override
        {
            if (m_raw != nullptr)
            {
                m_raw->on_message();
                delete m_raw;
            }
        }

    private:
        msg *m_raw;
    };

    auto wrapper = std::make_shared<WrappingMsg>(data);

    auto it = m_channels.find(target);
    if (it != m_channels.end() && it->second.writeQueue != nullptr)
    {
        bool ok = it->second.writeQueue->push(wrapper);
        if (ok)
        {
            target->wakeup();
        }
        return ok;
    }

    bool ok = target->m_ctrlQueue.push(wrapper);
    if (ok)
    {
        target->wakeup();
    }
    return ok;
}

void ThreadWrapper::post_msg(msg *data)
{
    ThreadWrapper *sender = ThreadWrapper::current();
    if (sender != nullptr && sender != this)
    {
        sender->post_msg(this, data);
        return;
    }

    class WrappingMsg : public msg_it
    {
    public:
        explicit WrappingMsg(msg *raw) : m_raw(raw)
        {
        }

        void on_message() override
        {
            if (m_raw != nullptr)
            {
                m_raw->on_message();
                delete m_raw;
            }
        }

    private:
        msg *m_raw;
    };

    auto wrapper = std::make_shared<WrappingMsg>(data);
    m_ctrlQueue.push(wrapper);
    wakeup();
}

void ThreadWrapper::wakeup()
{
    m_notify.signal();
}

void ThreadWrapper::register_to_driver()
{
    m_threadMgr->get_driver()->register_wrapper(this);
}
