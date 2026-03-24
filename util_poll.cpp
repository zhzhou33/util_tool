#pragma once

#include <sys/poll.h>
#include "util_poll.h"
#include "util_pipe.h"

void poll_t::loop()
{
    int32_t eventNums = ::poll(&(m_poll_fds[0]), m_poll_fds.size(), 10);

    if (eventNums <= 0)
        return;
    for (int i = 0; i < m_poll_fds.size(); ++i)
    {
        if (m_poll_fds[i].fd == -1)
            continue;
        if (m_poll_fds[i].revents == 0)
            continue;
        process_io_event(i, m_poll_fds[i].revents);
    }
}

void poll_t::process_io_event(uint32_t index, uint32_t masks)
{
    if (masks & POLLIN)
    {
        int8_t buff[] = {0};
        m_pipe[index].read(buff, sizeof(buff));
    }
    if (masks & POLLOUT)
    {
        int8_t buff[] = {0};
        m_pipe[index].write(buff, sizeof(buff));
    }
}
