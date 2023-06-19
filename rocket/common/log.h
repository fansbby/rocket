#ifndef ROCKET_COMMON_LOG_H
#define ROCKET_COMMON_LOG_H

#include <string>
#include <queue>
#include <memory>
#include <semaphore.h>

#include "rocket/common/config.h"
#include "rocket/common/mutex.h"
#include "rocket/net/timer_event.h"

namespace rocket{

template<typename... Args>
std::string formatString(const char* str,Args&&... args){
    int size = snprintf(nullptr,0,str,args...);
  
    std::string result;
    if(size>0){
        result.resize(size);
        snprintf(&result[0],size+1,str,args...);
    }
    

    return result;
}


#define DEBUGLOG(str, ...) \
  if (rocket::Logger::GetGlobalLogger()->getLogLevel() && rocket::Logger::GetGlobalLogger()->getLogLevel() <= rocket::Debug) \
  { \
    rocket::Logger::GetGlobalLogger()->pushLog(rocket::LogEvent(rocket::LogLevel::Debug).toString() \
      + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) + "\n");\
    rocket::Logger::GetGlobalLogger()->log(); \     
  }\


#define INFOLOG(str, ...) \
  if (rocket::Logger::GetGlobalLogger()->getLogLevel() <= rocket::Info) \
  { \
    rocket::Logger::GetGlobalLogger()->pushLog(rocket::LogEvent(rocket::LogLevel::Info).toString() \
    + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) + "\n");\
    rocket::Logger::GetGlobalLogger()->log(); \
  } \

#define ERRORLOG(str, ...) \
  if (rocket::Logger::GetGlobalLogger()->getLogLevel() <= rocket::Error) \
  { \
    rocket::Logger::GetGlobalLogger()->pushLog(rocket::LogEvent(rocket::LogLevel::Error).toString() \
      + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) + "\n");\
    rocket::Logger::GetGlobalLogger()->log(); \ 
  } \

/*
#define APPDEBUGLOG(str, ...) \
  if (rocket::Logger::GetGlobalLogger()->getLogLevel() <= rocket::Debug) \
  { \
    rocket::Logger::GetGlobalLogger()->pushAppLog(rocket::LogEvent(rocket::LogLevel::Debug).toString() \
      + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) + "\n");\
  } \


#define APPINFOLOG(str, ...) \
  if (rocket::Logger::GetGlobalLogger()->getLogLevel() <= rocket::Info) \
  { \
    rocket::Logger::GetGlobalLogger()->pushAppLog(rocket::LogEvent(rocket::LogLevel::Info).toString() \
    + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) + "\n");\
  } \

#define APPERRORLOG(str, ...) \
  if (rocket::Logger::GetGlobalLogger()->getLogLevel() <= rocket::Error) \
  { \
    rocket::Logger::GetGlobalLogger()->pushAppLog(rocket::LogEvent(rocket::LogLevel::Error).toString() \
      + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) + "\n");\
  } \
  */



//日志级别
enum LogLevel{
    Unknown = 0,
    Debug =1,
    Info = 2,
    Error = 3
};


//日志类
class Logger{
    public:
        typedef std::shared_ptr<Logger> s_ptr;

        Logger(LogLevel level) : m_set_level(level) {}

        void pushLog(const std::string& msg);

        void log();

        LogLevel getLogLevel() const {
            return m_set_level;
        }
    public:
        static Logger* GetGlobalLogger();
        static void InitGlobalLogger();
    private:
        LogLevel m_set_level;
        std::queue<std::string> m_buffer;

        Mutex m_mutex;

};


std::string LogLevelToString(LogLevel level);
LogLevel StringToLogLevel(const std::string &log_level);

//日志项
class LogEvent{
    public:
        LogEvent(LogLevel level): m_level(level){}

        
        std::string getFileName() const{
            return m_file_name;
        }

        LogLevel getLogLevel() const{
            return m_level;
        }
        std::string toString();

    private:
        std::string m_file_name; //文件名
        std::string m_file_line; //文件行
        int32_t m_pid; //进程号
        int32_t m_thread_id; //线程号
        std::string m_time; //时间

        LogLevel m_level;
        //Logger::s_ptr m_logger;
};




}





#endif