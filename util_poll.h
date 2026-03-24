#pragma once

#include "util_common.h"
#include "util_pipe.h"
#include <poll.h>
#include <sys/poll.h>

USING_UTIL_NAMESPACE
BEGIN_UTIL_NAMESPACE

class poll_t
{
public:
    void process_io_event(uint32_t index, uint32_t masks);

    void loop();

    void add(pollfd fd)
    {
        m_poll_fds.push_back(fd);
    }

    void add(pipe_t p)
    {
        m_pipe.push_back(p);
    }

public:
    std::vector<pollfd> m_poll_fds;
    std::vector<pipe_t> m_pipe;
};

END_UTIL_NAMESPACE
