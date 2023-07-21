#ifndef ROCKET_NET_STRING_CODER_H
#define ROCKET_NET_STRING_CODER_H

#include<string>
#include<vector>
#include"rocket/net/coder/abstract_protocol.h"
#include"rocket/net/coder/abstract_coder.h"



namespace rocket{

class StringProtocol:public AbstractProtocol{

public:
    std::string info; 

};



class StringCoder: public AbstractCoder{

    void encode(std::vector<AbstractProtocol::s_ptr> &messages,TcpBuffer::s_ptr out_buffer){
        for(size_t i=0;i<messages.size();++i){
            std::shared_ptr<StringProtocol> msg =std::dynamic_pointer_cast<StringProtocol>(messages[i]);
            out_buffer->writeToBuffer(msg->info.c_str(),msg->info.length());
        }
    }

    void decode(std::vector<AbstractProtocol::s_ptr> &out_messages,TcpBuffer::s_ptr buffer){
        std::vector<char> re;
        buffer->readFromBuffer(re,buffer->readAble());
        std::string info;
        for(size_t i=0;i<re.size();++i){
            info += re[i];
        }

        std::shared_ptr<StringProtocol> msg= std::make_shared<StringProtocol>();
        msg->info =info;
        msg->m_msg_id="123456";
        out_messages.push_back(msg);
    }


};









}



#endif