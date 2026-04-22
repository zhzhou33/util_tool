#include "test.h"
// #include "time_common.h"
#include "time_wheel.h"
#include "timer.h"

void Test::timer_work(const util_timer* who_is)
{
    auto time = get_now_time_stamp();

    std::cout << "timer_work who_is = " << (void*)who_is << ", time=" << time
              << std::endl;
}

void Test::func()
{
    auto time = get_now_time_stamp();
    // m_test_timer.add_timer(this, 2000, 1);
    m_test2_timer.add_timer(this, 1000, 10);

    // std::cout << "m_test_timer = " << &m_test_timer << ", time=" << time << std::endl;
    time = get_now_time_stamp();
    std::cout << "m_test2_timer = " << &m_test2_timer << ", time=" << time
              << std::endl;
}
