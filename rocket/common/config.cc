#include<tinyxml/tinyxml.h>
#include<string>

#include"rocket/common/config.h"

//读取配置文件的节点
#define READ_XML_NODE(name,parent) \
    TiXmlElement* name##_node = parent->FirstChildElement(#name); \ 
    if(!name##_node){ \
        printf("Start rocket server error, failed to read node[%s]",#name);\
        exit(0); \
    }  \

//读取配置文件内容，字符串
#define READ_STR_FROM_XML_NODE(name,parent) \
    TiXmlElement* name##_node = parent->FirstChildElement(#name); \
    if(!name##_node || !name##_node->GetText()){  \
        printf("Start rocket server error, failed to read node[%s]",#name);\
        exit(0); \
    } \
    std::string name##_str = std::string(name##_node->GetText()); \



namespace rocket{

static Config* g_config =NULL;


Config* Config::GetGlobalConfig(){
    return g_config;
}


void Config::SetGlobalConfig(const char* xmlfile){
    if(g_config == NULL)
        g_config = new Config(xmlfile);
    
}


Config::Config(const char* xmlfile){
    TiXmlDocument* xml_document = new TiXmlDocument();

    bool rt = xml_document->LoadFile(xmlfile);
    if(!rt){
        printf("Start rocket server error, failed to read xmlfile%s\n",xmlfile);
    }
    
    READ_XML_NODE(root,xml_document);
    READ_XML_NODE(log,root_node);
    
    READ_STR_FROM_XML_NODE(log_level,log_node);

    m_log_level = log_level_str;

}

} 