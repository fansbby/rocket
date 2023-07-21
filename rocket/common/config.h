#ifndef ROCKET_COMMON_CONFIG_H
#define ROCKET_COMMON_CONFIG_H

#include<string>


namespace rocket{

class Config
{

public:
    Config(const char* xmlfile);
    Config();
public:
    static Config* GetGlobalConfig();
    static void SetGlobalConfig(const char* xmlfile);

public:
    std::string m_log_level;

    std::string m_log_file_name; //日志输出文件名字
    std::string m_log_file_path; //日志输出路径
    int m_log_max_file_size{0};
    int m_log_sync_inteval{0};  //日志输出时间间隔

    int m_port;
    int m_io_threads;
};

} 



#endif