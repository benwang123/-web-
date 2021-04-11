#ifndef UTIL_H
#define UTIL_H

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <errno.h>
#include <memory.h>
#include <assert.h>

class Utils
{
public:
    Utils() {}
    ~Utils() {}

    //信号处理函数
    static void sig_handler(int sig);

    //设置信号函数
    void addsig(int sig, void(handler)(int), bool restart = true);

public:
    static int *u_pipefd;
    
};



#endif