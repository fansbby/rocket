#ifndef ROCKET_NET_FD_EVENT_GROUP_H
#define ROCKET_NET_FD_EVENT_GROUP_H


#include<vector>
#include"rocket/net/fd_event.h"
#include"rocket/common/mutex.h"

namespace rocket{


class FdEventGroup{

public:
    static FdEventGroup* getFdEventGroup();

public:
    FdEventGroup(int size);
    ~FdEventGroup();

    FdEvent* getFdEvent(int fd);

private:
    int m_size {0};
    std::vector<FdEvent*> m_fdevent_group;
    Mutex m_mutex;
};

    
}



#endif