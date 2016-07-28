#include"Memory_pool.h"
#include<stdio.h>
typedef Memorypool pool;
#define UNINT unsigned int


pool*pool::_pool=NULL;

Memorypool::Memorypool()
{
	init();
}

Memorypool::~Memorypool()
{
	close();
	pthread_mutex_destroy(&mutex);
}

void pool::init()
{
	_list=NULL;
	_last_list=NULL;
	pthread_mutex_init(&mutex,NULL);
}

void pool::close()
{
	//mutex
	pthread_mutex_lock(&mutex);
	mlist *plist=_list;
	while(plist!=NULL)
	{
		block *pblock=plist->_used;
		while(pblock!=NULL)
		{
			if(pblock->_brick!=NULL)
			{
				free(pblock->_brick);
				pblock->_brick=NULL;
			}
			pblock=pblock->_next;
		}
		pblock=plist->_free;
		while(pblock!=NULL)
		{
			if(pblock->_brick!=NULL)
			{
				free(pblock->_brick);
				pblock->_brick=NULL;
			}
			pblock=pblock->_next;
		}
		plist=plist->_next;
	}
	pthread_mutex_unlock(&mutex);
}


void *pool::set_pool_head(void *buff,mlist *list,block *block)
{
	if(buff==NULL)
	{
		return NULL;
	}

	UNINT *p=(UNINT*)buff;

	p[0]=(UNINT)list;
	p[1]=(UNINT)block;
	p[2]=(UNINT)CODE;

	return &p[3];  //从第13字节返回
	
}


void* pool::get_pool_head(void *buff)
{
	if(buff==NULL)
	{
		return NULL;
	}
	
	UNINT *p=(UNINT*)buff;

	return &p[3];
}

bool pool:: get_head_block(void *buff,mlist *list,block *pblock)
{
	char *save=(char *)buff;
	UNINT *p=(UNINT*)(save-MAX_MEMORYHEAD_SIZE);

	if(p[2]!=CODE)
	{
		return false;
	}

	list=(mlist*)p[0];
	pblock=(block*)p[1];
	return true;
}	

void *pool::getBuff(size_t buffsize)
{
	//mutex;
	
	pthread_mutex_lock(&mutex);
	void *pbuff=NULL;

	if(_list==NULL)
	{
		pbuff=malloc(buffsize+MAX_MEMORYHEAD_SIZE);
		if(pbuff==NULL)
		{
			pthread_mutex_unlock(&mutex);
			return NULL;
		}

		_list=(mlist*)malloc(sizeof(mlist));
		if(_list==NULL)
		{
			free(pbuff);
			pthread_mutex_unlock(&mutex);
			return NULL;
		}
		_list->init();

		block *pused=(block*)malloc(sizeof(block));
		if(pused==NULL)
		{
			free(pbuff);
			pthread_mutex_unlock(&mutex);
			return NULL;
		}
		pused->init();

		pused->_brick=pbuff;

		_list->_size=(int)buffsize;
		_list->_used=pused;
		_list->_usedlast=pused;
		
		_last_list=_list;

		pthread_mutex_unlock(&mutex);
		return set_pool_head(pbuff,_list,pused);
	}

	//查找大小合适的内存块
	mlist *plist=_list;
	while(plist!=NULL)
	{
		if(plist->_size=(int)buffsize)
		{
			block *pblock=plist->_free;
			
				if(pblock==NULL)//没有空闲的BLOCK
				{
					pbuff=malloc(buffsize+12);
					if(pbuff==NULL)
					{	
						pthread_mutex_unlock(&mutex);
						return NULL;
					}

					block *pused=(block*)malloc(sizeof(block));
					if(pused==NULL)
					{
						free(pbuff);
						pthread_mutex_unlock(&mutex);
						return NULL;
					}
					pused->init();
					pused->_brick=pbuff;

					block *pusedlast=plist->_usedlast;
				//	if(pusedlast==NULL)
				//	{
				//		plist->_usedlast=pused;
				//	}
					pused->_prev=pusedlast;
					pusedlast->_next=pused;
					pusedlast=pused;
					pthread_mutex_unlock(&mutex);
					return set_pool_head(pbuff,plist,pused);
				}
				else//有空闲的块
				{
					block *tmp=pblock;
					plist->_free=pblock->_next;
					pbuff=tmp->_brick;

					tmp->_prev=plist->_usedlast;
					tmp->_next=NULL;
					if(plist->_usedlast==NULL)//used为空
					{
						plist->_usedlast=tmp;
						plist->_used=tmp;
					}else{//used不为空
						plist->_usedlast->_next=tmp;
						plist->_usedlast=tmp;
					}
					return get_pool_head(pbuff);

				}
			}
		else
		{
			plist=plist->_next;
		}
	}
	pbuff=malloc(buffsize+12);
	if(pbuff==NULL)
	{
		pthread_mutex_unlock(&mutex);
		return NULL;
	}
	plist=(mlist*)malloc(sizeof(mlist));
	if(plist==NULL)
	{
		free(pbuff);
		pthread_mutex_unlock(&mutex);
		return NULL;
	}
	plist->init();
	block * pblock=(block*)malloc(sizeof(block));
	if(pblock==NULL)
	{
		free(pbuff);
		pthread_mutex_unlock(&mutex);
		return NULL;
	}
	pblock->init();

	pblock->_brick=pbuff;
	
	plist->_size=(int)buffsize;
	plist->_used=pblock;
	plist->_usedlast=pblock;
	
	_last_list->_next=plist;
	_last_list=plist;
	pthread_mutex_unlock(&mutex);
	return set_pool_head(pbuff,plist,pblock);
}


bool pool::delBuff(size_t buffsize,void *buff)
{
	//mutex
	pthread_mutex_lock(&mutex);
	mlist *plist=_list;
	while(plist!=NULL)
	{
		if(plist->_size==(int)buffsize)
		{
			block *pblock=plist->_used;
			bool first=true;
			while(pblock!=NULL)
			{
				if(pblock->_brick==buff)
				{
					if(pblock!=NULL)
					{
						if(first==true)
						{
							if(pblock->_next!=NULL)
							{
								pblock->_next->_prev=pblock->_prev;
							}
							plist->_used=pblock->_next;
							if(plist->_usedlast==pblock)
							{
								plist->_usedlast=plist->_usedlast->_prev;
							}
							first=false;
						}
						else
						{
							if(pblock->_next!=NULL)
							{
								pblock->_next->_prev=pblock->_prev;
							}
							if(plist->_usedlast==pblock)
							{
								plist->_usedlast=plist->_usedlast->_prev;
							}
							else
							{
								pblock->_prev->_next=pblock->_next;	
							}
						}
						if(plist->_free==NULL)
						{
							pblock->_prev=NULL;
							pblock->_next=NULL;
							plist->_free=pblock;
							plist->_usedlast=pblock;
						}
						else
						{
							pblock->_prev=plist->_freelast;
							pblock->_next=NULL;
							plist->_freelast->_next=pblock;
							plist->_freelast=pblock;
						}
						pthread_mutex_unlock(&mutex);
						return true;
					}
					else
					{
						pthread_mutex_lock(&mutex);
						return false;
					}
				}
				pblock=pblock->_next;
				first=false;
			}
		}
		plist=plist->_next;
	}
	pthread_mutex_unlock(&mutex);
	return false;
}

bool pool::delBuff(void *buff)
{
	//mutex;
	pthread_mutex_lock(&mutex);
	block *pblock=NULL;
	mlist *plist=NULL;
	if(get_head_block(buff,plist,pblock)==false)
	{
		pthread_mutex_unlock(&mutex);
		return false;
	}
	if(pblock!=NULL&&plist!=NULL)
	{
		if(plist->_used==pblock)
		{
			plist->_used=pblock->_next;
		}
		else
		{
			pblock->_prev->_next=pblock->_next;
		}
		
		if(pblock->_next!=NULL)
		{
			pblock->_next->_prev=pblock->_prev;
		}
		if(plist->_usedlast==pblock)
		{
			plist->_usedlast=plist->_usedlast->_prev;
		}
		if(plist->_free==NULL)
		{
			pblock->_prev=NULL;
			pblock->_next=NULL;
			plist->_free=pblock;
			plist->_freelast=pblock;
		}
		else
		{
			pblock->_next=NULL;
			pblock->_prev=plist->_usedlast;
			plist->_usedlast->_next=pblock;
			plist->_usedlast=pblock;
		}
		pthread_mutex_unlock(&mutex);
		return true;
	}
	else
	{
		pthread_mutex_unlock(&mutex);
		return false;
	}
}

void pool::displaypool()
{
	int usedCount=0;
	int freeCount=0;

	mlist *plist=_list;
	while(plist!=NULL)
	{
		block *used=plist->_used;
		block *free=plist->_free;

		usedCount=0;
		freeCount=0;

		while(used!=NULL)
		{
			usedCount++;
			used=used->_next;
		}
		printf("used Count=%d,size=%d\n",usedCount,plist->_size*usedCount);
		while(free!=NULL)
		{
			freeCount++;
			free=free->_next;
		}
		printf("free Count=%d,size=%d\n",freeCount,plist->_size*freeCount);
		plist=plist->_next;
	}
}

//MAX_MEMORYHEAD_SIZE 
//void *getBuff(size_t buffsize);
//		bool delBuff(size_t buffsize,void *buff);
//		bool delBuff(void *buff);
//		void displaypool();
//		pool();
//		void close();
//		void init();
//		void *pool::set_pool_head(void *buff,mlist *list,block *block);
//		void *get_pool_head(void *buff);
//		bool get_head_block(void *buff,mlist *list,block *block);
//	private:
//		static pool _pool;
//		mlist *_list;
//		mlist *_last_list;
//}
//
