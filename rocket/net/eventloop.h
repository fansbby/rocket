#ifndef ROCKET_COMMON_EVENTLOOP_H
#define ROCKET_COMMON_EVENTLOOP_H

#include <pthread.h>
#include <set>
#include <functional>
#include <queue>
#include "rocket/common/mutex.h"
#include "rocket/net/fd_event.h"
#include "rocket/net/wakeup_fd_event.h"
#include "rocket/net/timer.h"


namespace rocket{

class EventLoop{

public:
    EventLoop();
    ~EventLoop();

    void loop();

    void wakeup();

    void stop();

    void addEpollEvent(FdEvent* fdevent);

    void delEpollEvent(FdEvent* fdevent);

    bool isInLoopThread(); //判断当前是否有线程（是不是执行event loop的IO线程），如果是别的线程的话不能添加，因为有线程竞争

    //如果is_wake_up为true ，那么可以直接唤醒，赶快执行待执行任务，不用等待loop循环结束
    void addTask(std::function<void()> cb , bool is_wake_up =false);

    void addTimerEvent(TimerEvent::s_ptr event);

public:
    static EventLoop* GetCurrentEventloop();

private:
    void dealWakeUp();

    void InitWakeUpFdEvent();
    
    void InitTimer();

private:
    pid_t m_thread_id{0}; //线程号
    int m_epoll_fd{0};  //epoll fd
    int m_wakeup_fd{0};  //唤醒 fd 
    
    WakeUpFdEvent* m_wakeup_fd_event {NULL}; 

    bool m_stop_flag {false}; //循环退出flag

    std::set<int> m_listen_fds; //监听的fd

    std::queue<std::function<void()>> m_pending_tasks; //等待任务队列

    Mutex m_mutex; //互斥锁

    Timer* m_timer{NULL};
};


}
#endif