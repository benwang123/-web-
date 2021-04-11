#ifndef CONFIG_H
#define CONFIG_H

#include <fstream>
#include <map>

using namespace std;

typedef struct arg
{
    int SERVER_PORT;     //端口号
    int LOGWrite;        //日志写入方式
    int TRIGMode;        //触发组合模式
    int LISTENTrigmode;  //listenfd触发模式
    int CONNTrigmode;    //connfd触发模式
    int OPT_LINGER;      //优雅关闭链接

    int MYSQL_NUM;       //mysql连接池数量
    string MYSQL_ADDR;
    string MYSQL_USER;
    string MYSQL_PASSWD;
    string MYSQL_DB;
    int MYSQL_CLOSE_LOG;

    int REDIS_NUM;       //redis连接池数量
    string REDIS_ADDR;
    int REDIS_PORT;
    int REDIS_CLOSE_LOG;

    int THREAD_NUM;      //线程池内的线程数量
    int CLOSE_LOG;       //是否关闭日志
    int ACTOR_MODE;      //并发模型选择

}Arg_t;

class Config
{
private:
    string filename;
    map<string, string> m_map;

public:
    Config(string file);
    ~Config(){};
    bool ReadConfigFile();
    void RemoveSpace(string &str);
    void parsearg(Arg_t * arg);
    void init(Arg_t * _arg);
};


#endif