#ifndef ROCKET_NET_CODER_TINYPB_CODER_H
#define ROCKET_NET_CODER_TINYPB_CODER_H

#include<string>
#include"rocket/net/coder/abstract_coder.h"
#include"rocket/net/coder/tinypb_protocol.h"

namespace rocket{


struct TinyPBCoder:public AbstractCoder{

    //将message对象转化为字节流，写入到buffer
    void encode(std::vector<AbstractProtocol::s_ptr> &messages,TcpBuffer::s_ptr out_buffer);

    //将buffer 里面的字节流转换为message 对象
    void decode(std::vector<AbstractProtocol::s_ptr> &out_messages,TcpBuffer::s_ptr buffer);

    const char* encodeTinyPB(std::shared_ptr<TinyPBProtocol> message,int&len);

    TinyPBCoder(){}
    ~TinyPBCoder(){}
};


}

#endif