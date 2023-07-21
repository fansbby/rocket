#include<tinyxml/tinyxml.h>
#include<string>

#include"rocket/common/config.h"

//读取配置文件的节点
#define READ_XML_NODE(name,parent)\
    TiXmlElement* name##_node = parent->FirstChildElement(#name);\ 
    if(!name##_node){\
        printf("Start rocket server error, failed to read node[%s]",#name);\
        exit(0);\
    }\

//读取配置文件内容，字符串
#define READ_STR_FROM_XML_NODE(name,parent)\
    TiXmlElement* name##_node = parent->FirstChildElement(#name);\
    if(!name##_node || !name##_node->GetText()){\
        printf("Start rocket server error, failed to read node[%s]",#name);\
        exit(0);\
    } \
    std::string name##_str = std::string(name##_node->GetText());\



namespace rocket{

static Config* g_config =NULL;


Config* Config::GetGlobalConfig(){
    return g_config;
}


void Config::SetGlobalConfig(const char* xmlfile){
    if(g_config == NULL){
        if(xmlfile !=NULL){
            g_config = new Config(xmlfile);
        }else{
            g_config = new Config();
        }
    }
        
    
}


Config::Config(){
    m_log_level = "DEBUG";

}





Config::Config(const char* xmlfile){
    TiXmlDocument* xml_document = new TiXmlDocument();

    bool rt = xml_document->LoadFile(xmlfile);
    if(!rt){
        printf("Start rocket server error, failed to read xmlfile%s\n",xmlfile);
    }
    

    //读取配置文件
    READ_XML_NODE(root,xml_document);
    READ_XML_NODE(log,root_node);
    READ_XML_NODE(server,root_node);
    
    READ_STR_FROM_XML_NODE(log_level,log_node);
    READ_STR_FROM_XML_NODE(log_file_name,log_node);
    READ_STR_FROM_XML_NODE(log_file_path,log_node);
    READ_STR_FROM_XML_NODE(log_max_file_size,log_node);    
    READ_STR_FROM_XML_NODE(log_sync_inteval,log_node);

    m_log_level = log_level_str;

    m_log_file_name=log_file_name_str;
    m_log_file_path = log_file_path_str;
    m_log_max_file_size = std::atoi(log_max_file_size_str.c_str());
    m_log_sync_inteval = std::atoi(log_sync_inteval_str.c_str());  //日志输出时间间隔

    READ_STR_FROM_XML_NODE(port,server_node);
    READ_STR_FROM_XML_NODE(io_threads,server_node);
    m_port = std::atoi(port_str.c_str());
    m_io_threads = std::atoi(io_threads_str.c_str());

    printf("Server -- PORT[%d], IO Threads[%d]\n",m_port,m_io_threads);
}

} 