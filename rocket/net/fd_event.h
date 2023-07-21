#ifndef ROCKET_NET_FD_EVENT_H
#define ROCKET_NET_FD_EVENT_H

#include<sys/epoll.h>
#include<functional>

namespace rocket{

class FdEvent
{

public:
    enum TriggerEvent{
        IN_EVENT = EPOLLIN,
        OUT_EVENT = EPOLLOUT,
        ERROR_EVENT =EPOLLERR,
    };


    FdEvent(int fd);
    FdEvent();
    ~FdEvent();

    void setNonBlock();
    //根据事件类型返回对应函数
    std::function<void()> handler(TriggerEvent event_type);

    //获取监听函数
    void listen(TriggerEvent event_type,std::function<void()> cb, std::function<void()> error_callback =nullptr);

    // 取消监听
    void cancle(TriggerEvent event_type);

    int getFd() const{
        return m_fd;
    }

    epoll_event getEpollEvent(){
        return m_listen_events;
    }

    
protected:

    int m_fd {-1};
    epoll_event m_listen_events;

    std::function<void()> m_read_callback{nullptr};
    std::function<void()> m_write_callback{nullptr};
    std::function<void()> m_error_callback {nullptr};


};



}



#endif