#include "webserver.h"
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    Arg_t arg;
    //命令行解析,后续修改为利用配置参数进行传参
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