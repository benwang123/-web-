
#include "epoller.h"

int Epoll::maxEvent = 65535;

Epoll * Epoll::GetInstance()
{
	static Epoll _epoll;
	return &_epoll;
}

Epoll::Epoll():_events(maxEvent)
{
    //参数是多少都可以，只要大于0就好
    epollfd = epoll_create(5);
    if(epollfd <= 0)
    {
        exit(1);
    }
}

Epoll::~Epoll() 
{
    close(epollfd);
}

bool Epoll::AddFd(int fd,  bool oneshot, unsigned int event, int trigermode) 
{
    if(fd < 0) return false;
    epoll_event ev;
    ev.data.fd = fd;

    if (1 == trigermode)
        ev.events = event | EPOLLET | EPOLLRDHUP;
    else
        ev.events = event | EPOLLRDHUP;

    if (oneshot)
        ev.events |= EPOLLONESHOT;

    return 0 == epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

bool Epoll::ModFd(int fd,  bool oneshot, unsigned int event, int trigermode)
{
    if(fd < 0) return false;
    
    epoll_event ev;
    ev.data.fd = fd;

    if (1 == trigermode)
        ev.events = event | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    else
        ev.events = event | EPOLLONESHOT | EPOLLRDHUP;

    return 0 == epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}

bool Epoll::DelFd(int fd) 
{
    if(fd < 0) return false;
    epoll_event ev = {0};
    return 0 == epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}

int Epoll::Wait() 
{
    return epoll_wait(epollfd, &_events[0], static_cast<int>(_events.size()), -1);
}

int Epoll::GetEventFd(int i) const 
{
    if(i < _events.size() && i < 0)
    {
        exit(1);
    }
    return _events[i].data.fd;
}

int Epoll::GetEvents(int i) const 
{
    if(i > _events.size() && i < 0)
    {
        exit(1);
    }
    return _events[i].events;
}