#include<string.h>
#include"rocket/net/fd_event.h"



namespace rocket{

FdEvent::FdEvent(int fd) : m_fd(fd){
    memset(&m_listen_events,0,sizeof(m_listen_events));
}


FdEvent::FdEvent(){
    memset(&m_listen_events, 0, sizeof(m_listen_events));
}

FdEvent::~FdEvent(){
    

}

std::function<void()> FdEvent::handler(TriggerEvent event_type){
    if(event_type == TriggerEvent::IN_EVENT){
        return m_read_callback;
    }else{
        return m_write_callback;
    }
    return nullptr;
}


void FdEvent::listen(TriggerEvent event_type,std::function<void()> cb){
    if(event_type == TriggerEvent::IN_EVENT){
        m_listen_events.events |= EPOLLIN;  //如果是读事件，则给监听事件加一个读事件
        m_read_callback = cb;
    }else{
        m_listen_events.events |= EPOLLOUT; 
        m_write_callback = cb;
    }
    m_listen_events.data.ptr = this;
}

}