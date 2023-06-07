#include<pthread.h>
#include"rocket/common/log.h"
#include"rocket/common/config.h"


void* func(void*){
    DEBUGLOG("Debug is thread test %s","func");
    INFOLOG("Info is thread test %s","func");
    return NULL;
}




int main(){
    
    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();

    pthread_t thread;
    pthread_create(&thread,NULL,&func,NULL);

    DEBUGLOG("Debug test log %s","11");
    INFOLOG("Info test log %s","11");

    pthread_join(thread,NULL);
    return 0;
}