#include <cstddef>
#include <string>
#include <thread>

#include "test.h"
#include "thread_mgr.h"
#include "thread_wrapper.h"
#include "time_wheel.h"
#include "timer_driver.h"
#include "util.h"
#include "util_common.h"

USING_UTIL_NAMESPACE

void handle_user_event()
{
    Test *test = new Test();
    test->func();
}

class customer_msg : public util_msg
{
public:
    virtual void on_message()
    {
        std::cout << get_now_time_stamp() << " " << data << ", thread_id=" << std::this_thread::get_id()
                  << std::endl;
    }

public:
    std::string data;
};

int main()
{
    UTIL_LOG_INFO("hello " << "world");

    // ======== 模仿 tp_util 的架构 ========

    // 1. 获取线程管理器和驱动
    ThreadMgr *mgr = ThreadMgr::get_instance();

    // 2. 获取主线程
    IThreadWrapper *main_thr = mgr->get_main_thread();

    // 3. 创建 IO 线程
    IThreadWrapper *io_thr = mgr->create_thread(IO_THREAD);

    std::cout << "main thread id=" << std::this_thread::get_id() << std::endl;

    // 5. 添加定时器
    handle_user_event();

    for (int i = 0; i < 20; i++)
    {
        // 6. 发送消息到 IO 线程（通过 SPSC 通道，无锁）
        customer_msg *data = new customer_msg();
        auto curTime = get_now_time_stamp();
        data->data = std::to_string(curTime) + " hello from main (via SPSC channel)";
        io_thr->post_msg(data); // 当前线程(main) -> io_thr
    }

    // 7. 主循环
    while (true)
    {
        // 处理主线程消息
        main_thr->thread_run();
    }

    return 0;
}
