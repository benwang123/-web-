#include "redis_conn.h"

using namespace std;

string redis_conn::redis_addr  = "127.0.0.1";
int    redis_conn::redis_port  = 6379;
int    redis_conn::m_close_log = 0;
 
int redis_connection_pool::m_MaxConn = 8;

redis_conn::redis_conn() : redis_ins(NULL),reply(NULL),isconnect(false)
{
    
}

redis_conn::~redis_conn()
{
    redisFree(redis_ins); 
    redis_ins = NULL;
    reply = NULL;
}


bool redis_conn::Connect()
{
    redis_ins = redisConnect(redis_addr.c_str(), redis_port);
    if(redis_ins->err)
    {
        isconnect = false;
    }

    isconnect = true;
    return isconnect;
}

RedisStatus redis_conn::SendCmd(const string &cmd, RedisResult &res)
{
    reply = (redisReply*)redisCommand(redis_ins,cmd.c_str());
    return ParseReplay(res);
}

int redis_conn::FreeRedisReply()
{
    if (reply)
    {
        freeReplyObject(reply);
        reply = NULL;
    }
    return 0;
}

RedisStatus redis_conn::CheckErr()
{
    if(reply == NULL)
    {
        return _REPLY_ERROR;;
    }

    if(redis_ins == NULL)
    {
        return _CONTEXT_ERROR;
    }
    return _REDIS_OK;
}

RedisStatus redis_conn::ParseReplay(RedisResult &result)
{
    
    RedisStatus s = CheckErr();
    if(s != _REDIS_OK)
    {
        FreeRedisReply();
        return s;
    }

    switch(reply->type)
    {
        case REDIS_REPLY_STATUS:
            s = _REDIS_OK;
            result.type = reply->type;
            result.strdata = reply->str;
            break;
        case REDIS_REPLY_ERROR:
            s = _REPLY_ERROR;
            result.type = _CONNECT_FAIL;
            result.strdata =reply->str;
            //ZRedisBase::SetError(reply->str);
            break;
        case REDIS_REPLY_STRING:
            s = _REDIS_OK;
            result.type = reply->type;
            result.strdata = reply->str;
            break;
        case REDIS_REPLY_INTEGER:
            s = _REDIS_OK;
            result.type = reply->type;
            result.inter = reply->integer;
            break;
        case REDIS_REPLY_ARRAY:
            s = _REDIS_OK;
            result.type = reply->type;
            for (int i = 0; i < reply->elements; i ++)
            {
                if(reply->element[i]->type == REDIS_REPLY_NIL)
                {
                    result.vecdata.push_back(NULL);
                }
                else
                {
                    if(reply->element[i]->str==NULL)
                    {
                        result.vecdata.push_back("null");
                    }else{
                        result.vecdata.push_back(reply->element[i]->str);
                    }
                }

            }
            break;
        case REDIS_REPLY_NIL:
            s = _NIL_ERROR;
            result.type = reply->type;
            result.strdata = "REDIS_REPLY_NIL";
            if(reply->str == NULL)
            {
                //ZRedisBase::SetError("REDIS_REPLY_NIL");
            }else{
                //ZRedisBase::SetError(reply->str);
            }
            break;
        default:
            s = _REPLY_ERROR;
            result.type = reply->type;
            break;
    }

    FreeRedisReply();
    return s;
}

redis_connection_pool::redis_connection_pool()
{}

redis_connection_pool* redis_connection_pool::GetInstance()
{
    static redis_connection_pool redis_pool;
    return &redis_pool;
}

void redis_connection_pool::init()
{
    for (int i = 0; i < m_MaxConn; i++)
	{
		redis_conn * con = new redis_conn();
        if(con->Connect())
        {
            connList.push_back(con);
        }
        else
		{
			//LOG_ERROR("REDIS connect Error");
			exit(1);
		}
		
		++m_FreeConn;
		
	}
    reserve = sem(m_FreeConn);

}

redis_conn *redis_connection_pool::GetConnection()
{
	redis_conn *con = NULL;
    
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

bool redis_connection_pool::ReleaseConnection(redis_conn *con)
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

void redis_connection_pool::DestroyPool()
{
	lock.lock();
	if (connList.size() > 0)
	{
		for (auto it = connList.begin(); it != connList.end(); ++it)
		{
			auto con =  *it;
            delete con;
		}
		m_CurConn = 0;
		m_FreeConn = 0;
		connList.clear();
	}

	lock.unlock();
}

redis_connection_pool::~redis_connection_pool()
{
	DestroyPool();
}

int redis_connection_pool::GetFreeConn()
{
	return this->m_FreeConn;
}



