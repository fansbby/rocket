#ifndef ROCKET_NET_IO_THREAD_H
#define ROCKET_NET_IO_THREAD_H

#include<pthread.h>
#include<semaphore.h>
#include"rocket/net/eventloop.h"



namespace rocket{

class IOThread{

public:
    IOThread();
    ~IOThread();

public:
    static void* Main(void *arg);

    EventLoop* getEventLoop();

    void start();

    void join();

private:
    pid_t m_thread_id {-1}; //线程号

    pthread_t m_thread {0}; //进程号
  
    EventLoop* m_event_loop{NULL};

    sem_t m_init_semaphore; //信号量

    sem_t m_start_semaphore;
};

}

#endif