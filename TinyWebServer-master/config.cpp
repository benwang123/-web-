#include "config.h"
#include <iostream>

Config::Config(string file):filename(file){}

void Config::RemoveSpace(string &str)
{
    int start = 0, end = 0, pos = 0;
    if(str.empty())
    {
        return;
    }
    for(pos = 0;pos < str.size();pos++)
    {
        if(str[pos] == ' '|| str[pos] == '\t' || str[pos] == '\r')
        {
            continue;
        }
        else
        {
            break;			
        }
        
    }
    if(pos == str.size())
    {
        str = "";
        return;
    }

	start = pos;

    for(pos = str.size() - 1; pos > 0; pos--)
    {
        if(str[pos] == ' '|| str[pos] == '\t' || str[pos] == '\r') 
        {
            continue;
        }
        else
        {
            break;
        }
        
    }
    end = pos;
    str = str.substr(start, end - start + 1);

}

bool Config::ReadConfigFile()
{
    ifstream _file(filename);
    string line, key, value;
    int pos;
    if(!_file)
    {
        return false;
    }
    while(getline(_file,line))
    {
        //# 开头为注释
        if(line.empty()||line[0] == '#')
        {
            continue;
        }
        if((pos = line.find('=')) == string:: npos)
        {
            continue;
        }

        key   = line.substr(0, pos);
        value = line.substr(pos + 1, line.size() - pos -1);

        RemoveSpace(key);

        RemoveSpace(value);

        m_map.insert(pair<string,string>(key,value));
    }

}
void Config::parsearg(Arg_t * _arg)
{
    _arg->SERVER_PORT      = atoi(m_map["SERVER_PORT"].c_str());
    _arg->LOGWrite         = atoi(m_map["LOGWrite"].c_str());
    _arg->TRIGMode         = atoi(m_map["TRIGMode"].c_str());
    _arg->OPT_LINGER       = atoi(m_map["OPT_LINGER"].c_str());
    _arg->MYSQL_NUM        = atoi(m_map["MYSQL_NUM"].c_str());
    _arg->MYSQL_ADDR       = m_map["MYSQL_ADDR"];
    _arg->MYSQL_USER       = m_map["MYSQL_USER"];
    _arg->MYSQL_PASSWD     = m_map["MYSQL_PASSWD"];
    _arg->MYSQL_DB         = m_map["MYSQL_DB"];
    _arg->MYSQL_CLOSE_LOG  = atoi(m_map["MYSQL_CLOSE_LOG"].c_str());
    _arg->REDIS_NUM        = atoi(m_map["REDIS_NUM"].c_str());
    _arg->REDIS_ADDR       = m_map["REDIS_ADDR"];
    _arg->REDIS_PORT       = atoi(m_map["REDIS_PORT"].c_str());
    _arg->REDIS_CLOSE_LOG  = atoi(m_map["REDIS_CLOSE_LOG"].c_str());
    _arg->THREAD_NUM       = atoi(m_map["THREAD_NUM"].c_str());
    _arg->CLOSE_LOG        = atoi(m_map["close_log"].c_str());
    _arg->ACTOR_MODE       = atoi(m_map["actor_model"].c_str());
}   

void Config::init(Arg_t * arg)
{
	if(ReadConfigFile())
	{
		parsearg(arg);
	}
}