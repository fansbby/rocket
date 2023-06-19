#ifndef ROCKET_NET_UTIL_H
#define ROCKET_NET_UTIL_H


#include<sys/types.h>
#include<pthread.h>

namespace rocket{

    pid_t getPid();
    pid_t getThreadId();

    int64_t getNowMs();

}




#endif