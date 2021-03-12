#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H

#include <iostream>
#include <list>
#include <array>
#include <cstring>

using namespace std;

struct MemBlock
{
	void * buf = nullptr;;
	int state  = 0;       //该内存块正在使用为1，未使用为0
	int flag   = 0;       //记录该内存块属于哪一个结点
};


//内存结点的大小分别为64,128,256,512,1024,超过1024字节后直接从系统申请内存
struct MemPoint
{
	list<MemBlock *> _free_list;
	list<MemBlock *> _used_list;
	int free_num = 0;      //记录结点内空余内存块的个数   
};


//内存池管理器
class MempollMgr
{
private:

	array<MemPoint , 4> _pool;
	typedef array<MemPoint, 4>::iterator _iterator;

	_iterator _it;

private:

	MempollMgr()
	{
		_it = _pool.begin();
	}

	~MempollMgr()
	{
		for(_it;_it!=_pool.end();_it++)
		{
			auto list = _it ->_used_list;
			while(!list.empty())
			{
				auto p = list.front();
				free(p);
				list.pop_front();

				cout<<"dealloc"<<endl;
			}
			
			list = _it ->_free_list;
			while(!list.empty())
			{
				auto p = list.front();
				free(p);
				list.pop_front();
			}
		}
	}

	_iterator Mem_distribution(size_t size)
	{
		if(size <= 128)
		{
			return _it;
		}
		else if(128 < size <= 256)
		{
			return _it + 1;
		}
		else if(256 < size <= 384)
		{
			return _it + 2;
		}
		else if(384 < size <= 512)
		{
			return _it + 3;
		}
		else
		{

		}
	}

public:

	//单例模式，传引用
	static MempollMgr& Instance()
	{
		static MempollMgr _mempool;
		return _mempool;
	}

	void * alloc(size_t size)
	{
		
		_iterator it = Mem_distribution(size);
		int num = it - _pool.begin();
		//如果为空，代表该节点为空
		if(it -> _used_list.empty() && it -> _used_list.empty())
		{                                                
			MemBlock * block = (MemBlock *)malloc(sizeof(MemBlock) + ((num + 1) * 128));       //创建一个内存块
			block->flag  = num;
			block->state = 1;
			block->buf   = (char *)block + sizeof(MemBlock);
			it ->_used_list.push_back(block);                                                   //将内存块放入结点中                                       
			return block->buf;

		}
		else    //如果该节点存在
		{
			if(it -> free_num == 0)     //若没有空内存块
			{
				MemBlock * block = (MemBlock *)malloc(sizeof(MemBlock) + ((num + 1) * 128));

				block->flag  = num;
				block->state = 1;
				block->buf   = block + sizeof(MemBlock);

				it -> _used_list.push_back(block);
				return block->buf;
			}
			else
			{
				(it -> free_num)--;
				auto block = it -> _free_list.front();
				it -> _free_list.pop_front();
				
				block->state = 1;
				block->flag  = num;
				
				it -> _used_list.push_back(block);       //将即将使用的空内存块从链表头转移到链表的最后面
				
				return block->buf;
			}
		}
	}
	

	void dealloc(void * ptr)
	{
		char * pp = (char *)ptr;
		pp -= sizeof(MemBlock);

		MemBlock * p = (MemBlock*)pp;
		
		memset(ptr, 0 , (p -> flag + 1) * 128);

		auto iter = (_it + p -> flag)-> _used_list.begin();
		

		for(iter : (_it + p -> flag)-> _used_list)
		{
			if(iter -> state == 1)
			{
				break;
			}
		}

		(_it + p -> flag)-> _used_list.erase(iter);
		
		p->state = 0;
		(_it + p -> flag)-> _free_list.push_back(p);
	
	}
};




#endif
