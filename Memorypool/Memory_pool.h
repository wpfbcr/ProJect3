#pragma once

#include<iostream>
#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>


#define MAX_MEMORYHEAD_SIZE 12   //块地址+链表地址+4位校验码
#define CODE 0x941216            //校验码

typedef struct MemoryBlock{
	MemoryBlock* _next;
	MemoryBlock* _prev;
	void * _brick;

	MemoryBlock()
	{
		init();
	}

	void init()
	{
		_next=NULL;
		_prev=NULL;
		_brick=NULL;
	}
}block;//块结构


typedef struct MemoryList{
	MemoryList *_next;
	MemoryBlock *_free;
	MemoryBlock *_used;
	MemoryBlock *_freelast;
	MemoryBlock *_usedlast;

	int _size;

	MemoryList()
	{
		init();
	}

	void init()
	{
		_next=NULL;
		_free=NULL;
		_used=NULL;
		_freelast=NULL;
		_usedlast=NULL;
	}
}mlist;

class Memorypool
{
typedef Memorypool pool;
	public:
		static Memorypool& Instance()
		{
			if(_pool==NULL)
			{
				_pool=(pool*)malloc(sizeof(pool));
				_pool->init();
			}
			return *_pool;
		}
	public:
		~Memorypool();

		void *getBuff(size_t buffsize);
		bool delBuff(size_t buffsize,void *buff);
		bool delBuff(void *buff);
		void displaypool();
	private:
		Memorypool();
		void close();
		void init();
		void *set_pool_head(void *buff,mlist *list,block *block);
		void *get_pool_head(void *buff);
		bool get_head_block(void *buff,mlist *list,block *block);
	private:
		static Memorypool* _pool;
		mlist *_list;
		mlist *_last_list;
		pthread_mutex_t mutex;
};

