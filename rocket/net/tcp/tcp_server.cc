#include"rocket/net/tcp/tcp_server.h"
#include"rocket/common/log.h"



namespace rocket{

    TcpServer::TcpServer(NetAddr::s_ptr local_addr):m_local_addr(local_addr){
        init();
        INFOLOG("rocket TcpServer listen success on [%s]",m_local_addr->toString().c_str());
    }

    TcpServer::~TcpServer(){
        if(m_main_event_loop){
            delete m_main_event_loop;
            m_main_event_loop=NULL;
        }
    }

    void TcpServer::start(){
        m_io_thread_group->start();
        m_main_event_loop->loop();
    }

    void TcpServer::init(){
        m_acceptor =std::make_shared<TcpAcceptor>(m_local_addr);

        m_main_event_loop = EventLoop::GetCurrentEventloop();
        m_io_thread_group = new IOThreadGroup(2);

    
        m_listen_fd_event = new FdEvent(m_acceptor->getlistenfd());
        m_listen_fd_event->listen(FdEvent::IN_EVENT,std::bind(&TcpServer::onAccept,this));
        
        m_main_event_loop->addEpollEvent(m_listen_fd_event);


    }

    void TcpServer::onAccept(){
        int client_fd =m_acceptor->accept();
        //FdEvent client_fd_event(client_fd);
        ++m_client_count;

        //TODO:把clientfd 添加到任意io线程里面
        //m_io_thread_group->getIOThread()->getEventLoop()->addEpollEvent(client_fd_event);
        INFOLOG("TcpServer succ get client,fd =%d",client_fd);
    }
}
