#include "notify.h"
#include <chrono>

namespace util
{

notify::notify(int32_t ms)
    : m_pendingSignals(0)
    , m_waitMs(ms)
{
}

notify::~notify() = default;

int32_t notify::wait()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_waitMs < 0)
    {
        m_cv.wait(lock, [this] { return m_pendingSignals > 0; });
    }
    else
    {
        const bool ready = m_cv.wait_for(lock,
                                         std::chrono::milliseconds(m_waitMs),
                                         [this] { return m_pendingSignals > 0; });
        if (!ready)
        {
            return -1;
        }
    }

    --m_pendingSignals;
    return 0;
}

int32_t notify::signal()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        ++m_pendingSignals;
    }
    m_cv.notify_one();

    return 0;
}

}  // namespace util

