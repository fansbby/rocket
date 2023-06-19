#include <sys/timerfd.h>
#include <string.h>
#include <sys/unistd.h>
#include "rocket/net/timer.h"
#include "rocket/common/log.h"
#include "rocket/common/util.h"


namespace rocket{

    Timer::Timer() : FdEvent(){

        m_fd = timerfd_create(CLOCK_MONOTONIC , TFD_NONBLOCK | TFD_CLOEXEC);

        DEBUGLOG("timer fd = %d",m_fd);

        // 把fd的可读事件放到了eventloop上监听
        listen(FdEvent::IN_EVENT, std::bind(&Timer::onTimer,this));
    }

    Timer::~Timer(){

    }

    void Timer::onTimer(){
        //处理缓冲区数据，防止下一次继续触发可读事件
        DEBUGLOG("ontimer");
        char buf[8];
        while(1){
            if((read(m_fd,buf,8) == -1)&&errno == EAGAIN){
                break;
            }
        }

        int64_t now = getNowMs();
        std::vector<TimerEvent::s_ptr> tmps;
        std::vector<std::pair<int64_t,std::function<void()>>> tasks;

        ScopeMutext<Mutex> lock(m_mutex);

        //删除已经过期的任务，把他们拷贝到新建的临时数组中
        auto it = m_pending_events.begin();
        for(it=m_pending_events.begin();it!=m_pending_events.end();++it){
            if((*it).first <= now){
                if(!(*it).second->isCancled()){
                    tmps.push_back((*it).second);
                    tasks.push_back(std::make_pair((*it).second->getArriveTime(),(*it).second->getCallBack()));
                }
            }else{
                break;
            }
        }

        m_pending_events.erase(m_pending_events.begin(),it);
        lock.unlock();


        //把重复的event再添加进去
        for(auto i=tmps.begin();i!=tmps.end();++i){
            if((*i)->isRepeated()){
                //如果重复的，重新调整arrivetime
                (*i)->resetArriveTime();
                addTimerEvent((*i));
            }
        }

        //保险起见，重新设置一次时间
        resetArriveTime();

        //执行所有的定时任务，只要到期了都会去执行
        for(auto i:tasks){
            if(i.second){
                i.second();
            }
        }  


    }



    void Timer::resetArriveTime(){
        ScopeMutext<Mutex> lock(m_mutex);
        auto tmp = m_pending_events;
        lock.unlock();

        if(!tmp.size()){
            return ;
        }

        auto it = tmp.begin();  //获取最开始的定时任务时间
        int64_t now = getNowMs();  
        int64_t timeval = 0;
        if(it->second->getArriveTime() > now){
            timeval = it->second->getArriveTime()-now;
        }else{
            timeval = 100;   //最开始的定时任务已经过期了，100ms后再次执行
        }

        timespec ts;
        memset(&ts,0,sizeof(ts));
        ts.tv_sec = timeval /1000;
        ts.tv_nsec = (timeval %1000) * 1000000;

        itimerspec value;
        memset(&value,0,sizeof(value));
        value.it_value = ts;

        int rt = timerfd_settime(m_fd,0,&value,NULL);
        if(rt != 0){
            ERRORLOG("timefd_settime error , errno=%d,error=%s",errno,strerror(errno));
        }
        DEBUGLOG("timer reset to%lld",now + timeval);
    }


    void Timer::addTimerEvent(TimerEvent::s_ptr event){
        bool is_reset_timefd = false;

        ScopeMutext<Mutex> lock(m_mutex);
        if(m_pending_events.empty()){
            is_reset_timefd = true;
        }else{
            auto it = m_pending_events.begin();
            if((*it).second->getArriveTime() > event->getArriveTime()){
                is_reset_timefd = true;
            }
        }
        m_pending_events.emplace(event->getArriveTime(),event);
        lock.unlock();

        if(is_reset_timefd){
            resetArriveTime();
        }
    }

    void Timer::deleteTimerEvent(TimerEvent::s_ptr event){

        ScopeMutext<Mutex> lock(m_mutex);
        
        auto begin = m_pending_events.upper_bound(event->getArriveTime());
        auto end = m_pending_events.lower_bound(event->getArriveTime());

        for(auto it =begin;it!=end;++it){
            if(it->second == event){
                m_pending_events.erase(it);
            }
        }
        lock.unlock();

    }
}