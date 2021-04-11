#ifndef _REDIS_POOL_
#define _REDIS_POOL_

#include <iostream>
#include <hiredis/hiredis.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <vector>
#include <list>
#include "../lock/locker.h"
#include "../log/log.h"
#include "../CGImysql/sql_connection_pool.h"

using namespace std;

//redis 的执行状态
enum RedisStatus
{
    _REDIS_OK = 0, //执行成功
    _CONNECT_FAIL = -1, //连接redis失败
    _CONTEXT_ERROR = -2, //RedisContext返回错误
    _REPLY_ERROR = -3, //redisReply错误
    _XE_COMMAND_ERROR = -4, //redis命令执行错误
    _NIL_ERROR = -5 //不存在访问的数据
};

typedef struct st_redisResult
{
    int type;
    size_t inter;
    string strdata;
    vector<std::string> vecdata;
}RedisResult;


class redis_conn
{
public:
    redis_conn();
    ~redis_conn();
    bool Connect();
    //send cmd
    RedisStatus SendCmd(const std::string &cmd, RedisResult &res);
    //parse content
    RedisStatus ParseReplay(RedisResult &result);
    // chect errnum
    RedisStatus CheckErr();
    int FreeRedisReply();

public:
    static string redis_addr;
    static int    redis_port;
    static int    m_close_log;
private:
    redisContext * redis_ins;
    redisReply   * reply;

    bool isconnect; 

};

class redis_connection_pool:public connection_pool<redis_conn>
{
public:
    redis_conn * GetConnection();		       //获取数据库连接
	bool ReleaseConnection(redis_conn *conn);  //释放连接
	int GetFreeConn();					       //获取连接
	void DestroyPool();					       //销毁所有连接
    void init();

public:
    static redis_connection_pool* GetInstance();
    static int m_MaxConn;

private:
    redis_connection_pool();
    ~redis_connection_pool();
    list<redis_conn *> connList; //连接池
	sem reserve;
    locker lock;
	int m_CurConn;  //当前已使用的连接数
	int m_FreeConn; //当前空闲的连接数
};


#endif