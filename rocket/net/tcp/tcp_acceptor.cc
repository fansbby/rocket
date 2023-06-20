#include<assert.h>
#include<fcntl.h>
#include<string.h>
#include"rocket/common/log.h"
#include"rocket/net/tcp/net_addr.h"
#include"rocket/net/tcp/tcp_acceptor.h"



namespace rocket{
    TcpAcceptor::TcpAcceptor(NetAddr::s_ptr loacl_addr):m_loacl_addr(loacl_addr){
        if(!loacl_addr->checkValid()){
            ERRORLOG("invalid local addr%s",loacl_addr->toString().c_str());
            exit(0);
        }

        m_family = m_loacl_addr->getFamily();

        m_listenfd = socket(m_family,SOCK_STREAM,0);

        if(m_listenfd <0){
            ERRORLOG("invalid listenfd %d",m_listenfd);
            exit(0);
        }

        int val =1;
        
        if(setsockopt(m_listenfd,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val)) !=0){ //SO_REUSEADDR表示tcp断连后，可以复用之前的端口号，而不会被占用，可以让服务器快速重启
            ERRORLOG("setsockopt REUSEADDR error, errno=%d , error =%s",errno,strerror(errno));
        }

        socklen_t len = m_loacl_addr->getSockLen();
        if(bind(m_listenfd,m_loacl_addr->getSockAddr(),len) !=0){
            ERRORLOG("bind error, errno=%d , error =%s",errno,strerror(errno));
            exit(0);
        }

        if(listen(m_listenfd,1000)!=0){
            ERRORLOG("listen error, errno=%d , error =%s",errno,strerror(errno));
            exit(0);
        }

    }

    TcpAcceptor::~TcpAcceptor(){
        
    }

    int TcpAcceptor::accept(){
        if(m_family ==AF_INET){  //ipv4协议
            sockaddr_in client_addr;
            memset(&client_addr,0,sizeof(client_addr));
            socklen_t client_addr_len = sizeof(client_addr);

            int client_fd = ::accept(m_listenfd,reinterpret_cast<sockaddr*>(&client_addr),&client_addr_len);
            if(client_fd<0){
                ERRORLOG("accept error, errno=%d , error =%s",errno,strerror(errno));
                exit(0);
            }
            IPNetAddr peer_addr(client_addr);
            INFOLOG("A client have accepted succ, peer addr [%s]",peer_addr.toString().c_str());
            return client_fd;
        }else{
            //...
        }
    }

    int TcpAcceptor::getlistenfd(){
        return m_listenfd;  
    }

}