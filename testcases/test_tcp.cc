#include"rocket/net/tcp/net_addr.h"
#include"rocket/common/log.h"
#include"rocket/net/tcp/tcp_server.h"

void test_tcpserver(){
    rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1",12346);

    DEBUGLOG("creatr addr %s",addr->toString().c_str());

    rocket::TcpServer tcp_server(addr);
    
    tcp_server.start();
}





int main(){
    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    
    rocket::Logger::InitGlobalLogger();

    //rocket::IPNetAddr addr("127.0.0.1",12345);
    //DEBUGLOG("create succ addr %s",addr.toString().c_str());

    test_tcpserver();
    return 0;
}