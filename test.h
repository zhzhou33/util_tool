#pragma once

#include "timer.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

USING_UTIL_NAMESPACE

inline std::string date_time(std::time_t posix)
{
    char buf[20];
    std::tm tp = *std::localtime(&posix);
    return std::string(buf, std::strftime(buf, sizeof(buf), "%F %T", &tp));
}

inline std::string stamp()
{
    using namespace std;
    using namespace std::chrono;

    // get absolute wall time
    auto now = system_clock::now();

    // find the number of milliseconds
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    // build output string
    std::ostringstream oss;
    oss.fill('0');

    // convert absolute time to time_t seconds
    // and convert to "date time"
    oss << date_time(system_clock::to_time_t(now));
    oss << "." << std::setw(3) << ms.count();

    return oss.str();
}

class Test : public util_timer_sink
{
public:
    Test() {}
    ~Test() {}
    virtual void timer_work(const util_timer* who_is);

    void func();

private:
    util_timer m_test_timer;
    util_timer m_test2_timer;
};
