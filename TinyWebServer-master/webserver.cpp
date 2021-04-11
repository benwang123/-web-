#include "webserver.h"

#define TIMEOUT 30

WebServer::WebServer()
{
    //http_conn类对象
   // users = new http_conn[MAX_FD];

    //root文件夹路径
    char server_path[200]; 
    getcwd(server_path, 200);
    char root[6] = "/root";
    m_root = (char *)MempollMgr::Instance().alloc(strlen(server_path) + strlen(root) + 1);
    strcpy(m_root, server_path);
    strcat(m_root, root);

    //users_timer = new client_data[MAX_FD];
}

WebServer::~WebServer() 
{
    close(m_listenfd);
    close(m_pipefd[1]);
    close(m_pipefd[0]);

    for(auto it = _users.begin();it != _users.end();it++)
    {
        delete it->second;
    }

    _users.clear();
}

void WebServer::init(Arg_t * arg)
{
    m_port       = arg->SERVER_PORT;
    m_log_write  = arg->LOGWrite;
    m_OPT_LINGER = arg->OPT_LINGER;
    m_TRIGMode   = arg->TRIGMode;
    m_close_log  = arg->CLOSE_LOG;
    m_actormodel = arg->ACTOR_MODE;

    mysql_conn::addr         = arg->MYSQL_ADDR;
    mysql_conn::user         = arg->MYSQL_USER;
    mysql_conn::passwd       = arg->MYSQL_PASSWD;
    mysql_conn::db           = arg->MYSQL_DB;
	mysql_conn::m_close_log  = arg->MYSQL_CLOSE_LOG;

    mysql_connection_pool::m_MaxConn = arg->MYSQL_NUM;

    redis_conn::redis_addr = arg->REDIS_ADDR;
    redis_conn::redis_port = arg->REDIS_PORT;

    redis_connection_pool::m_MaxConn = arg->REDIS_NUM;

    threadpool<http_conn>::thread_number = arg->THREAD_NUM;
    threadpool<http_conn>::actor_mode    = arg->ACTOR_MODE;
    
    trig_mode();

    epoller = Epoll::GetInstance();
    //定时器
    timer   = HeapTimer::GetInstance();
    //初始化数据库连接池
    mysql_connection_pool::GetInstance()->init();
    //创建redis连接池
    redis_connection_pool::GetInstance()->init();
    //线程池
    threadpool<http_conn>::GetInstance()->init();
    
}

void WebServer::trig_mode()
{
    //LT + LT
    if (0 == m_TRIGMode)
    {
        m_LISTENTrigmode = 0; 
        m_CONNTrigmode = 0;
    }
    //LT + ET
    else if (1 == m_TRIGMode)
    {
        m_LISTENTrigmode = 0;
        m_CONNTrigmode = 1;
    }
    //ET + LT
    else if (2 == m_TRIGMode)
    {
        m_LISTENTrigmode = 1;
        m_CONNTrigmode = 0;
    }
    //ET + ET
    else if (3 == m_TRIGMode)
    {
        m_LISTENTrigmode = 1;
        m_CONNTrigmode = 1;
    }
}

void WebServer::closeconn(http_conn * conn)
{
    assert(conn);
    epoller->DelFd(conn->getfd());
    conn->close_conn();

    //删除对应的连接
    delete _users[conn->getfd()];
    _users.erase(conn->getfd());

    LOG_INFO("Client[%d] quit!", conn->getfd());
}

void WebServer::log_write()
{
    if (0 == m_close_log)
    {
        //初始化日志
        if (1 == m_log_write)
            Log::get_instance()->init("./ServerLog", m_close_log, 2000, 800000, 800);
        else
            Log::get_instance()->init("./ServerLog", m_close_log, 2000, 800000, 0);
    }
}

//对文件描述符设置非阻塞
int WebServer::setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void WebServer::eventListen()
{
    //网络编程基础步骤
    m_listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(m_listenfd >= 0);

    //优雅关闭连接
    if (0 == m_OPT_LINGER)
    {
        struct linger tmp = {0, 1};
        setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }
    else if (1 == m_OPT_LINGER)
    {
        struct linger tmp = {1, 1};
        setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(m_port);

    int flag = 1;
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    ret = bind(m_listenfd, (struct sockaddr *)&address, sizeof(address));
    assert(ret >= 0);
    ret = listen(m_listenfd, 5);
    assert(ret >= 0);

    //监听
    epoller->AddFd(m_listenfd, false, EPOLLIN, m_LISTENTrigmode);

    setnonblocking(m_listenfd);
    setnonblocking(m_pipefd[1]);
    setnonblocking(m_pipefd[0]);

    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_pipefd);
    assert(ret != -1);
    epoller->AddFd(m_pipefd[0], false, EPOLLIN, 0);

    utils.addsig(SIGPIPE, SIG_IGN);
    utils.addsig(SIGALRM, utils.sig_handler, false);
    utils.addsig(SIGTERM, utils.sig_handler, false);

    //定时触发报警信号
    alarm(TIMEOUT);

    //工具类,信号和描述符基础操作
    Utils::u_pipefd = m_pipefd;
}

bool WebServer::dealclinetdata()
{
    struct sockaddr_in client_address;
    socklen_t client_addrlength = sizeof(client_address);
    if (0 == m_LISTENTrigmode)
    {
        int connfd = accept(m_listenfd, (struct sockaddr *)&client_address, &client_addrlength);
        if (connfd < 0)
        {
            LOG_ERROR("%s:errno is:%d", "accept error", errno);
            return false;
        }
        if (http_conn::m_user_count >= MAX_FD)
        {
            close(connfd);
            LOG_ERROR("%s", "Internal server busy");
            return false;
        }

        http_conn * conn = new http_conn();
        conn->init(connfd, client_address,epoller);

        epoller->AddFd(connfd, true, EPOLLIN, 1);
        _users[connfd] = conn;
        setnonblocking(connfd);

        timer->add(connfd, TIMEOUT,bind(&WebServer::closeconn,this, _users[connfd]));
    }

    else
    { 
        while (1)
        {
            int connfd = accept(m_listenfd, (struct sockaddr *)&client_address, &client_addrlength);
            if (connfd < 0)
            {
                LOG_ERROR("%s:errno is:%d", "accept error", errno);
                break;
            }
            if (http_conn::m_user_count >= MAX_FD)
            {
                close(connfd);
                LOG_ERROR("%s", "Internal server busy");
                break;
            }

            http_conn * conn = new http_conn();
            conn->init(connfd, client_address, epoller);

            epoller->AddFd(connfd, true, EPOLLIN, 1);
            _users[connfd] = conn;
            setnonblocking(connfd);

            timer->add(connfd, TIMEOUT,bind(&WebServer::closeconn, this, _users[connfd]));
        }
        return false;
    }
    return true;
}

bool WebServer::dealwithsignal(bool &timeout, bool &stop_server)
{
    int ret = 0;
    int sig;
    char signals[1024];
    ret = recv(m_pipefd[0], signals, sizeof(signals), 0);
    if (ret == -1)
    {
        return false;
    }
    else if (ret == 0)
    {
        return false;
    }
    else
    {
        for (int i = 0; i < ret; ++i)
        {
            switch (signals[i])
            {
                case SIGALRM:
                {
                    timeout = true;
                    break;
                }
                case SIGTERM:
                {
                    stop_server = true;
                    break;
                }
            }
        }
    }
    return true;
}

void WebServer::dealwithread(int sockfd)
{
    //调整定时器
    timer->adjust(sockfd, 3 * TIMEOUT);
    //reactor
    if (1 == m_actormodel)
    {

        //若监测到读事件，将该事件放入请求队列
        threadpool<http_conn>:: GetInstance()->append(_users[sockfd], 0);

        while (true)
        {
            if (1 == _users[sockfd]->improv)
            {
                if (1 == _users[sockfd]->timer_flag)
                {
                    timer->doWork(sockfd);
                    _users[sockfd]->timer_flag = 0;
                }
                _users[sockfd]->improv = 0;
                break;
            }
        }
    }
    else
    {
        //proactor
        if (_users[sockfd]->read_once())
        {
            LOG_INFO("deal with the client(%s)", inet_ntoa(_users[sockfd]->get_address()->sin_addr));

            //若监测到读事件，将该事件放入请求队列
             threadpool<http_conn>:: GetInstance()->append_p(_users[sockfd]);  
        }
        else
        {
            timer->doWork(sockfd);
        }
    }
}

void WebServer::dealwithwrite(int sockfd)
{
    timer->adjust(sockfd, 3 * TIMEOUT);
    //reactor
    if (1 == m_actormodel)
    {
         threadpool<http_conn>:: GetInstance()->append(_users[sockfd], 1);

        while (true)
        {
            if (1 == _users[sockfd]->improv)
            {
                if (1 == _users[sockfd]->timer_flag)
                {
                    timer->doWork(sockfd);
                    _users[sockfd]->timer_flag = 0;
                }
                _users[sockfd]->improv = 0;
                break;
            }
        }
    }
    else
    {
        //proactor
        if (_users[sockfd]->write())
        {
            LOG_INFO("send data to the client(%s)", inet_ntoa(_users[sockfd]->get_address()->sin_addr));
        }
        else
        {
            timer->doWork(sockfd);
        }
    }
}

void WebServer::eventLoop()
{
    bool timeout = false;
    bool stop_server = false;

    while (!stop_server)
    {
        int number = epoller->Wait();
        if (number < 0 && errno != EINTR)
        {
            LOG_ERROR("%s", "epoll failure");
            break;
        }

        for (int i = 0; i < number; i++)
        {
            int sockfd = epoller->GetEventFd(i);
            int events = epoller->GetEvents(i);

            //处理新到的客户连接
            if (sockfd == m_listenfd)
            {
                //std::cout<<"connect"<<endl;
                bool flag = dealclinetdata();
                if (false == flag)
                    continue;
            }
            else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                //服务器端关闭连接，移除对应的定时器
                timer->doWork(sockfd);
            }
            //处理信号
            else if ((sockfd == m_pipefd[0]) && (events & EPOLLIN))
            {
            	//读取timeout与stop_server的状态
                bool flag = dealwithsignal(timeout, stop_server);
                if (false == flag)
                    LOG_ERROR("%s", "dealclientdata failure");
            }
            //处理客户连接上接收到的数据
            else if (events & EPOLLIN)
            {
                dealwithread(sockfd);
            }
            else if (events & EPOLLOUT)
            {
                dealwithwrite(sockfd);
            }
        }
		//收到SIGALARM信号
        if (timeout)
        {
            timer->tick();
            alarm(TIMEOUT);
            LOG_INFO("%s", "timer tick");
            timeout = false;
        }
    }
}