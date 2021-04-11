#include "webserver.h"
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    Arg_t arg;
   
    Config config("webserver.cfg");

    config.init(&arg);

    WebServer server;

    //初始化
    server.init(&arg);

    //日志
    server.log_write();
   
    //监听
    server.eventListen();

    //运行
    server.eventLoop();

    return 0;
}