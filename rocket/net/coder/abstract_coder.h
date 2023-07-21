#ifndef ROCKET_NET_ABSTACT_CODER_H
#define ROCKET_NET_ABSTACT_CODER_H


#include<vector>
#include<memory>
#include"rocket/net/tcp/tcp_buffer.h"
#include"rocket/net/coder/abstract_protocol.h"


namespace rocket{


class AbstractCoder{

public:
    typedef std::shared_ptr<AbstractCoder> s_ptr;

public:

    //将message对象转化为字节流，写入到buffer
    virtual void encode(std::vector<AbstractProtocol::s_ptr> &messages,TcpBuffer::s_ptr out_buffer) =0;

    //将buffer 里面的字节流转换为message 对象
    virtual void decode(std::vector<AbstractProtocol::s_ptr> &out_messages,TcpBuffer::s_ptr buffer) =0;

    virtual ~AbstractCoder(){}
};



}

#endif