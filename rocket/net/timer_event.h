#ifndef ROCKET_NET_TIME_EVENT_H
#define ROCKET_NET_TIME_EVENT_H

#include<functional>
#include<memory>



namespace rocket{

class TimerEvent
{


public:
    typedef std::shared_ptr<TimerEvent> s_ptr;

    TimerEvent(int interval, bool is_repeated,std::function<void()> cb);

    ~TimerEvent();

    int64_t getArriveTime(){
        return m_arrive_time;
    }

    void setCancled(bool val){
        m_is_cancled = val;
    }

    bool isCancled(){
        return m_is_cancled;
    }

    bool isRepeated(){
        return m_is_repeated;
    }

    void resetArriveTime();

    std::function<void()> getCallBack(){
        return m_task;
    }

private:
    int64_t m_arrive_time; //ms
    int64_t m_interval;  //ms
    bool m_is_repeated{false};
    bool m_is_cancled{false};


    std::function<void()> m_task;
};




}



#endif