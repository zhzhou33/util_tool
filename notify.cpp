#include "notify.h"
#include <sys/poll.h>

int32_t notify::wait()
{
    pollfd fd_set;
    fd_set.fd = m_pipe.get_read_handle();
    fd_set.events = POLLIN;

    int32_t rv = ::poll(&fd_set, 1, m_waitMs);
    if (rv == 0)
        return -1;
    if (fd_set.revents & POLLIN)
    {
        m_pipe.read(&m_sig, sizeof(m_sig));
    }

    return 0;
}

int32_t notify::signal()
{
    m_pipe.write(&m_sig, sizeof(m_sig));
    return 0;
}

