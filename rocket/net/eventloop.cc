#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/unistd.h>
#include <string.h>
#include "rocket/net/eventloop.h"
#include "rocket/common/log.h"
#include "rocket/common/util.h"


//如果fd已经在监听队列里面了，那么只用修改，否则是添加
#define ADD_TO_EPOLL() \
    auto it = m_listen_fds.find(event->getFd()); \
    int op = EPOLL_CTL_ADD; \
    if (it != m_listen_fds.end()) { \
      op = EPOLL_CTL_MOD; \
    } \
    epoll_event tmp = event->getEpollEvent(); \
    INFOLOG("epoll_event.events = %d", (int)tmp.events); \
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp); \
    if (rt == -1) { \
      ERRORLOG("failed epoll_ctl when add fd, errno=%d, error=%s", errno, strerror(errno)); \
    } \
    m_listen_fds.insert(event->getFd()); \
    DEBUGLOG("add event success, fd[%d]", event->getFd()) \


#define DELETE_TO_EPOLL() \
    auto it = m_listen_fds.find(event->getFd()); \
    if (it == m_listen_fds.end()) { \
      return; \
    } \
    int op = EPOLL_CTL_DEL; \
    epoll_event tmp = event->getEpollEvent(); \
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), NULL); \
    if (rt == -1) { \
      ERRORLOG("failed epoll_ctl when add fd, errno=%d, error=%s", errno, strerror(errno)); \
    } \
    m_listen_fds.erase(event->getFd()); \
    DEBUGLOG("delete event success, fd[%d]", event->getFd()); \


/*修改一处BUG，把if-else中else的条件移出来*/

namespace rocket{


    static thread_local EventLoop* t_current_eventloop = NULL;
    static int g_epoll_max_timeout =100000;
    static int g_epoll_max_events =10;
    EventLoop::EventLoop(){
        if(t_current_eventloop !=NULL){
            ERRORLOG("falied to create event loop ,this thread has created event loop");
            exit(0);
        }
        m_thread_id = getThreadId();
        //INFOLOG("succ create event loop in thread %d", m_thread_id);
        
        m_epoll_fd = epoll_create(10);
        if(m_epoll_fd ==-1){
            ERRORLOG("failed to create event loop, epoll_create error,error info[%d]",errno);
            exit(0);
        }


        //创建wakeup
        InitWakeUpFdEvent();
        InitTimer();

        INFOLOG("succ create event loop in thread %d",m_thread_id);
        t_current_eventloop = this;


    }

    EventLoop::~EventLoop(){
        close(m_epoll_fd);
        if(m_wakeup_fd_event){
            delete m_wakeup_fd_event;
            m_wakeup_fd_event = NULL;
        }
        if(m_timer){
            delete m_timer;
            m_timer = NULL;
        }
    }


     void EventLoop::InitTimer(){
        m_timer = new Timer();
        addEpollEvent(m_timer);
     }

    void EventLoop::addTimerEvent(TimerEvent::s_ptr event){
        m_timer->addTimerEvent(event);
    }

    void EventLoop::InitWakeUpFdEvent(){
        //创建唤醒fd
        m_wakeup_fd = eventfd(0,EFD_NONBLOCK);
        if(m_wakeup_fd <0){
            ERRORLOG("failed to create event loop, eventfd error,error info[%d]",errno);
            exit(0);
        }


        INFOLOG("wakeup fd = %d", m_wakeup_fd);
        //创建wakeup事件
        m_wakeup_fd_event = new WakeUpFdEvent(m_wakeup_fd);

        m_wakeup_fd_event->listen(FdEvent::IN_EVENT,[this](){
            char buf[8];
            while(read(m_wakeup_fd, buf, 8) != -1 && errno !=EAGAIN){ // 因为是非阻塞的，所以可以一直读 ，直到返回-1或者错误不等于EAGAIN
                
            }
            DEBUGLOG("read full bytes from wakeup fd[%d]",m_wakeup_fd);
        }
        );
        addEpollEvent(m_wakeup_fd_event);
    }


    void EventLoop::loop(){
        m_is_looping = true;
        while(!m_stop_flag){
            ScopeMutex<Mutex> lock(m_mutex);
            std::queue<std::function<void()>> tmp_tasks ;  //加锁保证线程安全
            m_pending_tasks.swap(tmp_tasks);
            lock.unlock();

            while(!tmp_tasks.empty()){
                std::function<void()>cb = tmp_tasks.front(); //直接执行队列中的回调函数
                tmp_tasks.pop();
                if(cb){
                    cb();
                }
            }

            // 如果有定时任务需要执行，那么执行
            // 1. 怎么判断一个定时任务需要执行？ （now() > TimerEvent.arrtive_time）
            // 2. arrtive_time 如何让 eventloop 监听

            int time_out = g_epoll_max_timeout;
            epoll_event result_event[g_epoll_max_events];
            //DEBUGLOG("now begin to epoll_wait");
            int rt = epoll_wait(m_epoll_fd,result_event,g_epoll_max_events,time_out);
            DEBUGLOG("now end epoll_wait,rt=%d",rt);
            if(rt<0){
                ERRORLOG("epoll_wait error, errno=%d, error=%s", errno, strerror(errno));
            }else{
                for(int i=0;i<rt;++i){
                    epoll_event trigger_event =result_event[i];
                    FdEvent* fd_event = static_cast<FdEvent*> (trigger_event.data.ptr);
                    if(fd_event ==NULL){
                        ERRORLOG("fd_event = NULL, continue");
                        continue;
                    }

                    //int event =(int)(trigger_event.events);
                    //DEBUGLOG("unknow event =%d",event);

                    if(trigger_event.events & EPOLLIN){
                        DEBUGLOG("fd %d trigger EPOLLIN event", fd_event->getFd());
                        addTask(fd_event->handler(FdEvent::IN_EVENT));
                    }
                    if(trigger_event.events & EPOLLOUT){
                        DEBUGLOG("fd %d trigger EPOLLOUT event", fd_event->getFd());
                        addTask(fd_event->handler(FdEvent::OUT_EVENT));
                    }

                    if(trigger_event.events & EPOLLERR){
                        DEBUGLOG("fd %d trigger EPOLLERROR event", fd_event->getFd());
                        //删除出错的套接字
                        delEpollEvent(fd_event);
                        if(fd_event->handler(FdEvent::ERROR_EVENT)!=nullptr){
                            DEBUGLOG("fd %d add error callback",fd_event->getFd());
                            addTask(fd_event->handler(FdEvent::OUT_EVENT));
                        }
                        
                    }

                }
            }
        }
    }

    void EventLoop::wakeup(){
        INFOLOG("WAKE UP");
        m_wakeup_fd_event->wakeup();
    }

    void EventLoop::stop(){
        m_stop_flag = true;
        wakeup();
    }

    void EventLoop::dealWakeUp(){

    }

    /*如果是当前的线程直接执行就行，如果不是epoll的主线程执行的，那么会判断为false，会把函数添加到待执行队列，
    等当前的IO线程执行epollloop完返回后，再去执行任务队列里面的回调函数*/
    void EventLoop::addEpollEvent(FdEvent* event){
        if(isInLoopThread()){
            ADD_TO_EPOLL();
        }else{
            auto cb = [this,event] (){
                ADD_TO_EPOLL();
            };
            addTask(cb,true);
        }
    }

    void EventLoop::delEpollEvent(FdEvent* event){
        if(isInLoopThread()){
            DELETE_TO_EPOLL();
        }else{
            auto cb = [this,event] (){
                DELETE_TO_EPOLL();
            };
            addTask(cb,true);
        }
    }

    bool EventLoop::isInLoopThread(){
        return getThreadId() == m_thread_id;
    }

    void EventLoop::addTask(std::function<void()> cb , bool is_wake_up /*=false*/){
        ScopeMutex<Mutex> lock(m_mutex);
        m_pending_tasks.push(cb);
        lock.unlock();
        if(is_wake_up){    
            wakeup();
        }
    }

    EventLoop* EventLoop::GetCurrentEventloop(){
        if(t_current_eventloop){
            return t_current_eventloop;
        }
        t_current_eventloop = new EventLoop();
        return t_current_eventloop;
    }

    bool EventLoop::isLooping(){
        return m_is_looping;
    }

}