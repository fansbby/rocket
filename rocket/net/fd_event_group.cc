#include "rocket/net/fd_event_group.h"
#include "rocket/common/mutex.h"
#include "rocket/common/log.h"

namespace rocket{

    static FdEventGroup* g_fd_event_group = NULL;


    FdEventGroup* FdEventGroup::getFdEventGroup(){
        if(g_fd_event_group!=NULL){
            return g_fd_event_group;
        }
        g_fd_event_group = new FdEventGroup(128);
        return g_fd_event_group;
    }

    FdEventGroup::FdEventGroup(int size):m_size(size){
        for(int i=0;i<m_size;++i){
            m_fdevent_group.push_back(new FdEvent(i));
        }
    }
    FdEventGroup::~FdEventGroup(){
        for(int i=0;i<m_size;++i){
            if(m_fdevent_group[i]!=NULL){
                delete m_fdevent_group[i];
                m_fdevent_group[i]=NULL;
            }
        }
    }

    FdEvent* FdEventGroup::getFdEvent(int fd){
        ScopeMutex<Mutex> lock(m_mutex);
        if((size_t) fd < m_fdevent_group.size()){
            return m_fdevent_group[fd];
        }
        int new_size = int (fd*1.5);
        for(int i = m_fdevent_group.size();i<new_size;++i){
            m_fdevent_group.push_back(new FdEvent(i));
        }
        return m_fdevent_group[fd];
    }




    
}