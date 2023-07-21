#include <string.h>
#include <fcntl.h>
#include "rocket/net/fd_event.h"
#include "rocket/common/log.h"


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
  if (event_type == TriggerEvent::IN_EVENT) {
    return m_read_callback;
  } else if (event_type == TriggerEvent::OUT_EVENT) {
    return m_write_callback;
  } else if (event_type == TriggerEvent::ERROR_EVENT) {
    return m_error_callback;
  }
  return nullptr;
}


void FdEvent::listen(TriggerEvent event_type,std::function<void()> cb,std::function<void()> error_callback /*=nullptr*/){
    if(event_type == TriggerEvent::IN_EVENT){
        m_listen_events.events |= EPOLLIN;  //如果是读事件，则给监听事件加一个读事件
        m_read_callback = cb;
    }else{
        m_listen_events.events |= EPOLLOUT; 
        m_write_callback = cb;
    }
    
    if(m_error_callback == nullptr){
        m_error_callback = error_callback;
    }else{
        m_error_callback = nullptr;
    }

    m_listen_events.data.ptr = this;
}

void FdEvent::setNonBlock(){
    int flag =fcntl(m_fd,F_GETFL,0);
    if(flag & O_NONBLOCK){
        return ;
    }
    fcntl(m_fd,F_SETFL,flag | O_NONBLOCK);
}

void FdEvent::cancle(TriggerEvent event_type) {
  if (event_type == TriggerEvent::IN_EVENT) {
    m_listen_events.events &= (~EPOLLIN);
  } else {
    m_listen_events.events &= (~EPOLLOUT);
  }
}

}