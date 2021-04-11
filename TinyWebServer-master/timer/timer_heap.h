#ifndef TIMER_HEAP_H
#define TIMER_HEAP_H

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <assert.h>
#include <functional>

#include <time.h>
#include <vector>
#include <unordered_map>
#include "../log/log.h"
#include "../http/http_conn.h"

using namespace std;

//定时器，网络库中三大部分，事件，定时器，信号处理
//定时器的主要作用在于判断链接是否超时

/*
定时器有两种常用模式

一次性定时器，设置在某个时间点触发一次，可以称为 run_after

周期性定时器，需要周期性触发，可以称为 run_every

周期性定时器，只不过是该次timer触发以后，需要再次添加一个timer到管理器中以待下次触发。

具体流程：

	1 从数据结构中取出所有需要触发的timer，注意：将这些timer从all_timer中删除

	2 调用timer处理函数

	3 添加周期性定时器，以待下次触发

	4 调整定时器的下次超时时间

*/

typedef function<void()> TimeoutCallBack;

typedef struct timerNode 
{
    int id;
    int expires;
    TimeoutCallBack cb_func;
    bool operator<(timerNode& t) 
    {
        return expires < t.expires;
    }
}timernode_t;

class HeapTimer 
{
private:
    HeapTimer() { }

    ~HeapTimer() { clear(); }

public:

    static HeapTimer * GetInstance();
    
    void adjust(int id, int newExpires);

    void add(int id, int timeOut, TimeoutCallBack cb);

     void del(int i);

    void doWork(int id);

    void clear();

    void tick();

    void pop();

private:
    
    void siftup_(int i);

    bool siftdown_(int index, int n);

    void SwapNode_(int i, int j);

    vector<timernode_t> heap;

    unordered_map<int, int> ref;

};






#endif
