#pragma once

#include <cstdint>
#include <cstddef>
#include <netdb.h>
#include "util_common.h"

BEGIN_UTIL_NAMESPACE

template <class T>
class RingBuffer : public NoCopy
{
public:
    RingBuffer(uint32_t capacity = 32)
        : m_header(nullptr)
        , m_read(nullptr)
        , m_write(nullptr)
        , m_capacity(capacity)
    {
        m_header = new element[m_capacity];
        m_read = m_write = m_header;
        for (uint32_t idx = 0; idx < m_capacity - 1; ++idx)
        {
            m_header[idx].m_next = &m_header[idx + 1];
        }
        m_header[m_capacity - 1].m_next = m_read;
    }

    virtual ~RingBuffer() { delete[] m_header; }

    bool empty() const { return m_read == m_write; }
    bool full() const { return m_read == m_write->m_next; }
    uint32_t capacity() const { return m_capacity; }

    int32_t push(T& value)
    {
        if (this->full())
        {
            return -1;
        }
        m_write->m_val = value;
        m_write = m_write->m_next;
        return 0;
    }

    int32_t pop(T& value)
    {
        if (this->empty())
        {
            return -1;
        }
        value = m_read->m_val;
        m_read = m_read->m_next;
        return 0;
    }

    uint32_t size()
    {
        if (this->empty())
            return 0;
        if (this->full())
            return m_capacity - 1;
        uint32_t size = 0;
        element* start = m_read;
        element* end = m_write;
        if (start <= end)
            size = (uint32_t)(end - start);
        else
            size = (uint32_t)((end - m_header) + (m_header[m_capacity - 1] - start) + 1);

        return size;
    }

public:
    typedef T value_type;
    struct element
    {
        value_type m_val;
        element* m_next;

        element(element* next = nullptr)
            : m_val(value_type())
            , m_next(next)
        {
        }
    };

private:
    element* m_header;
    element* m_read;
    element* m_write;
    uint32_t m_capacity;
};

class msg
{
public:
    virtual ~msg() = default;
    virtual void on_message() = 0;
};

END_UTIL_NAMESPACE
