#ifndef ROCKET_NET_ABSTACT_PROROCOL_H
#define ROCKET_NET_ABSTACT_PROROCOL_H

#include<memory>
#include<string>

namespace rocket{

class AbstractProtocol:public std::enable_shared_from_this<AbstractProtocol>{
public:
    typedef std::shared_ptr<AbstractProtocol> s_ptr;

public:
    virtual ~AbstractProtocol() {}


    std::string m_msg_id;
};



}





#endif