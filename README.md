#### 1 日志模块开发
首先创建项目：

日志模块：
```
1.日志级别，
2.打印到文件，支持日期命名，以及日志的滚动，
3.c格式化风控
4.线程安全
```

LogLevel:
```
Debug
Info
Error
```

LogEvent:
```
文件名、行号
MsgNo
进程号
Thread id
日期 时间 精确到ms
自定义消息
```

日志格式
```
[level][%y-%m-%d %H:%M:%s.%ms]\t[pid:thread_id]\t[file_name:line][%msg]
```

Logger 日志器
1.提供打印日志的方法
2.设置日志输出的路径

### 2 Reactor

Reactor，又可以称为EventLoop，它的本质是一个事件循环模型。
Reactor，核心逻辑是一个loop循环，使用伪代码描述如下：
```c++{.line-numbers}
void loop() {
  while(!stop) {
      foreach (task in tasks) {
        task();
      }

      // 1.取得下次定时任务的时间，与设定time_out去较大值，即若下次定时任务时间超过1s就取下次定时任务时间为超时时间，否则取1s
      int time_out = Max(1000, getNextTimerCallback());
      // 2.调用Epoll等待事件发生，超时时间为上述的time_out
      int rt = epoll_wait(epfd, fds, ...., time_out); 
      if(rt < 0) {
          // epoll调用失败。。
      } else {
          if (rt > 0 ) {
            foreach (fd in fds) {
              // 添加待执行任务到执行队列
              tasks.push(fd);
            }
          }
      }      
  }
}


```

在 rocket 里面，使用的是主从 Reactor 模型，如下图所示：
![](./imgs/main-sub-reactor.drawio.png)
服务器有一个mainReactor和多个subReactor。

mainReactor由主线程运行，他作用如下：通过epoll监听listenfd的可读事件，当可读事件发生后，调用accept函数获取clientfd，然后随机取出一个subReactor，将cliednfd的读写事件注册到这个subReactor的epoll上即可。也就是说，mainReactor只负责建立连接事件，不进行业务处理，也不关心已连接套接字的IO事件。

subReactor通常有多个，每个subReactor由一个线程来运行。subReactor的epoll中注册了clientfd的读写事件，当发生IO事件后，需要进行业务处理。

### 2.1 TimerEvent 定时任务
```
1. 指定时间点 arrive_time
2. interval, ms 时间间隔。
3. is_repeated 周期性
4. is_cancled 
5. task 


cancle()
cancleRepeated()
```
### 2.2 Timer 定时器
```
addTimerEvent();
deleteTimerEvent();

onTimer();    // 当发生了 IO 事件之后，需要执行的方法


reserArriveTime()

multimap 存储 TimerEvent <key(arrivetime), TimerEvent>
```

### 2.3 IO线程
创建一个IO 线程，他会帮我们执行：

1. 创建一个新线程（pthread_create）
2. 在新线程里面 创建一个 EventLoop，完成初始化
3. 开启 loop

```c++{.line-numbers}
class {
  pthread_t m_thread;
  pid_t m_thread_id;
  EventLoop event_loop;
}
```