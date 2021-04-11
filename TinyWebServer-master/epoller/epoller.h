#ifndef EPOLL_H
#define EPOLL_H

#include <sys/epoll.h> 
#include <fcntl.h>  
#include <unistd.h> 
#include <vector>
#include <errno.h>
#include <stdlib.h>


class Epoll 
{
private:
    explicit Epoll();

    ~Epoll();
public:

    bool AddFd(int fd,  bool oneshot, unsigned int event, int trigermode);

    bool ModFd(int fd,  bool oneshot, unsigned int event, int trigermode);

    bool DelFd(int fd);

    int Wait();

    int GetEventFd(int i) const;

    int GetEvents(int i) const;

    static int maxEvent;

    static Epoll * GetInstance();
        
private:
    int epollfd;

    std::vector<struct epoll_event> _events;    
};





#endif 