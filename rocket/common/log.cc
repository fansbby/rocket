#include <sys/time.h>
#include <sstream>
#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <iostream>
#include "rocket/common/log.h"
#include "rocket/common/util.h"
#include "rocket/common/config.h"
#include "rocket/net/eventloop.h"



namespace rocket{

    static Logger* g_logger = NULL;

    Logger* Logger::GetGlobalLogger(){
        return g_logger;
    }

    void Logger::InitGlobalLogger(){
        LogLevel global_log_level = StringToLogLevel(Config::GetGlobalConfig()->m_log_level);
        printf("Init log level[%s]\n",LogLevelToString(global_log_level).c_str());
        g_logger = new Logger(global_log_level);
    }

    std::string LogLevelToString(LogLevel level){
        switch (level)
        {
        case Debug:
            return "DEBUG";
        case Info:
            return "INFO";
        case Error:
            return "ERROR";
        default:
            return "UNKNOW";
        }
    }

    LogLevel StringToLogLevel(const std::string &log_level){
        if(log_level == "DEBUG"){
            return Debug;
        }else if(log_level == "INFO"){
            return Info;
        }else if(log_level == "ERROR"){
            return Error;
        }else{
            return Unknown;
        }
    }



    //打印日志  [%y-%m-%d %H:%M:%s.%ms]\t[pid:thread_id]\t[file_name:line][%msg] 日志格式
    std::string LogEvent::toString(){
        struct timeval now_time;

        gettimeofday(&now_time, nullptr);

        struct tm now_time_t;
        localtime_r(&(now_time.tv_sec), &now_time_t);

        char buf[128];
        strftime(&buf[0], 128, "%y-%m-%d %H:%M:%S", &now_time_t);
        std::string time_str(buf);
        int ms = now_time.tv_usec / 1000;
        time_str = time_str + "." + std::to_string(ms);


        m_pid = getPid();
        m_thread_id = getThreadId();

        std::stringstream ss;
        ss << "[" << LogLevelToString(m_level) << "]\t"
            << "[" << time_str << "]\t"
            << "[" << m_pid << ":" << m_thread_id << "]\t";
          
        
        std::cout<<ss.str();
        return ss.str();
    }

    void Logger::pushLog(const std::string& msg){

        ScopeMutext<Mutex> lock(m_mutex);
        m_buffer.push(msg);
        lock.unlock();
        
    }

     //打印到终端
    void Logger::log(){

        ScopeMutext<Mutex> lock(m_mutex);
        std::queue<std::string> tmp;
        m_buffer.swap(tmp);
        lock.unlock();
        while(!tmp.empty()){
            std::string msg = tmp.front();
            tmp.pop();
            printf(msg.c_str());
        }
    }
}