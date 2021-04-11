#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>
#include <unordered_map>

//#include "./epoller/epoller.h"
#include "./threadpool/threadpool.h"
#include "./memory/mempool.h"
#include "./config.h"
#include "./timer/timer_heap.h"
#include "./util/util.h"

const int MAX_FD = 65536;           //最大文件描述符
const int MAX_EVENT_NUMBER = 10000; //最大事件数
const int TIMESLOT = 5;             //最小超时单位

class WebServer
{
public:
    WebServer();
    ~WebServer();

    void init(Arg_t * arg);

    void log_write();
    void trig_mode();
    void eventListen();
    void eventLoop();
    bool dealclinetdata();
    bool dealwithsignal(bool& timeout, bool& stop_server);
    void dealwithread(int sockfd);
    void dealwithwrite(int sockfd);
    int  setnonblocking(int fd);

private:
    //基础
    int m_port;
    char *m_root;
    int m_log_write;
    int m_actormodel;

    int m_listenfd;
    int m_OPT_LINGER;
    int m_TRIGMode;
    int m_LISTENTrigmode;
    int m_CONNTrigmode;

    Epoll * epoller;
    unordered_map<int, http_conn *> _users;
    HeapTimer * timer;

    void closeconn(http_conn * conn);

    void Add(int a);

public:

    int m_pipefd[2]; 
    int m_close_log;

    Utils utils;
};

#endif
