#ifndef _CONNECTION_POOL_
#define _CONNECTION_POOL_

#include <stdio.h>
#include <list>
#include <vector>
#include <mysql/mysql.h>
#include <error.h>
#include <string.h>
#include <iostream>
#include <string>
#include "../lock/locker.h"
#include "../log/log.h"

using namespace std;

template<typename T>
class connection_pool
{
public:
    connection_pool()
    {
		
	}
    virtual ~connection_pool()
    {}
public:
    virtual void init() = 0;

    virtual T * GetConnection() = 0;		      //获取数据库连接
	virtual bool ReleaseConnection(T *conn) = 0;  //释放连接
	virtual int GetFreeConn() = 0;				  //获取连接
	virtual void DestroyPool() = 0;	
};

class mysql_conn
{
public:
    mysql_conn();
    ~mysql_conn();
    bool Connect();
    //send cmd
    bool SendCmd(char * mysql_cmd);

    vector< vector<string> > ParseContent();

public:
    static string addr;
    static string user;
    static string passwd;
    static string db;
	static int m_close_log;

private:

    MYSQL  * conn;
    MYSQL_RES * res;
    bool isconnect; 
};


class mysql_connection_pool:public connection_pool<mysql_conn>
{
public:
	mysql_conn * GetConnection();				 //获取数据库连接
	bool ReleaseConnection(mysql_conn *conn); //释放连接
	int GetFreeConn();					 //获取连接
	void DestroyPool();					 //销毁所有连接

	static mysql_connection_pool *GetInstance();

	void init();
	static int m_MaxConn;

private:
	mysql_connection_pool();
	~mysql_connection_pool();

	int m_CurConn;  //当前已使用的连接数
	int m_FreeConn; //当前空闲的连接数
	locker lock;
	list<mysql_conn *> connList; 
	sem reserve;
};

template <typename T1, typename T2>
class connectionRAII
{
public:
	connectionRAII(T1 * &con, T2 *connPool);
	~connectionRAII();
	
private:
	T1 *conRAII;
	T2 *poolRAII;
};

template <typename T1, typename T2>
connectionRAII<T1, T2>::connectionRAII(T1 * &con, T2 *connPool)
{
	con = connPool->GetConnection();
	
	conRAII = con;
	poolRAII = connPool;
}

template <typename T1, typename T2>
connectionRAII<T1,T2>::~connectionRAII()
{
	poolRAII->ReleaseConnection(conRAII);
}

#endif
