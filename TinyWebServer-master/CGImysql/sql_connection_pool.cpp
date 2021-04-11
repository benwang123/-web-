#include <mysql/mysql.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <list>
#include <pthread.h>
#include <iostream>
#include "sql_connection_pool.h"

using namespace std;

string mysql_conn::addr     = "192.168.1.6";
string mysql_conn::user     = "root";
string mysql_conn::passwd   = "154263";
string mysql_conn::db       = "han";
int mysql_conn::m_close_log = 0;

int mysql_connection_pool::m_MaxConn   = 8;

mysql_conn::mysql_conn()
{
	conn = NULL;
	res  = NULL;
	isconnect = false;
}

mysql_conn::~mysql_conn()
{
	mysql_close(conn);
	res  = NULL;
	isconnect = false;
}

bool mysql_conn::Connect()
{
	MYSQL * mysql_con = new MYSQL;
	if(!mysql_init(mysql_con))
	{
		LOG_ERROR("MySQL init Error");
		exit(1);
	}

	this->conn = mysql_real_connect(mysql_con, addr.c_str(), user.c_str(), passwd.c_str(), db.c_str(), 3306, NULL, 0);
	if(conn == NULL)
	{
		LOG_ERROR("MySQL connect Error is %s",mysql_error(mysql_con));
		exit(1);
	}

	isconnect = true;

	return isconnect;
}

bool mysql_conn::SendCmd(char * mysql_cmd)
{
	if(conn != NULL)
	{
		if (mysql_query(conn, mysql_cmd))
		{
			LOG_ERROR("MySQL send data error , is %s",mysql_error(conn));
			return false;
		}
		else
		{
			return true;
		}	
	}
	else
	{
		LOG_ERROR("MySQL fd is NULL");
		exit(1);
	}
	
}

vector< vector<string> > mysql_conn::ParseContent()
{
	vector< vector<string> > m_vector;
	res = mysql_store_result(conn);
	if(res != NULL)
	{
		int num_fields = mysql_num_fields(res);

		while (MYSQL_ROW row = mysql_fetch_row(res))
		{
			vector<string> v1;
			for(int i = 0; i < num_fields; i++)
			{
				string tmp(row[i]);
				v1.push_back(tmp);
			}
			m_vector.push_back(v1);
		}
	}
	else
	{
		LOG_ERROR("MySQL recv data error");
	}

	return m_vector;
	
}

mysql_connection_pool::mysql_connection_pool()
{
	m_CurConn = 0;
	m_FreeConn = 0;
}

mysql_connection_pool *mysql_connection_pool::GetInstance()
{
	static mysql_connection_pool connPool;
	return &connPool;
}

void mysql_connection_pool::init()
{
	for (int i = 0; i < m_MaxConn; i++)
	{
		mysql_conn * con = new mysql_conn();
        if(con->Connect())
        {
            connList.push_back(con);
        }
       
		++m_FreeConn;
		
	}
    reserve = sem(m_FreeConn);
}

//当有请求时，从数据库连接池中返回一个可用连接，更新使用和空闲连接数
mysql_conn *mysql_connection_pool::GetConnection()
{
	mysql_conn *con = NULL;
    
	if (0 == connList.size())
		return NULL;

	reserve.wait();
	
	lock.lock();

	con = connList.front();
	connList.pop_front();

	--m_FreeConn;
	++m_CurConn;

	lock.unlock(); 
	return con;
}

//释放当前使用的连接
bool mysql_connection_pool::ReleaseConnection(mysql_conn *con)
{
	if (NULL == con)
		return false;

	lock.lock();

	connList.push_back(con);
	++m_FreeConn;
	--m_CurConn;

	lock.unlock();

	reserve.post();
	return true;
}

//销毁数据库连接池
void mysql_connection_pool::DestroyPool()
{

	lock.lock();
	if (connList.size() > 0)
	{
		for (auto it = connList.begin(); it != connList.end(); ++it)
		{
			mysql_conn *con = *it;
			delete con;
		}
		m_CurConn = 0;
		m_FreeConn = 0;
		connList.clear();
	}

	lock.unlock();
}

//当前空闲的连接数
int mysql_connection_pool::GetFreeConn()
{
	return this->m_FreeConn;
}

mysql_connection_pool::~mysql_connection_pool()
{
	DestroyPool();
}
