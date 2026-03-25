#pragma once

#include "util_common.h"
#include <cstring>
#include <unistd.h>
#include <cstdint>

USING_UTIL_NAMESPACE
BEGIN_UTIL_NAMESPACE

class pipe_t
{
public:
    typedef int32_t pipe_type;

public:
    pipe_t()
    {
        std::memset(m_pipe_handle, 0, sizeof(m_pipe_handle));
        open();
    }

    virtual ~pipe_t() {}

    int32_t open()
    {
        return ::pipe(m_pipe_handle);
    }

    int32_t read(void* buff, uint32_t count)
    {
        return ::read((int)m_pipe_handle[0], buff, count);
    }

    int32_t write(void* buff, uint32_t count)
    {
        return ::write((int)m_pipe_handle[1], buff, count);
    }

    void close() {}

    pipe_type get_read_handle() const
    {
        return m_pipe_handle[0];
    }

    pipe_type get_write_handle() const
    {
        return m_pipe_handle[1];
    }

private:
    pipe_type m_pipe_handle[2];
};

END_UTIL_NAMESPACE

