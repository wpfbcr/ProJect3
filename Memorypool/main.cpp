#include"Memory_pool.h"
#include<string.h>
#include<stdio.h>

int main(void)
{

	Memorypool p=Memorypool::Instance();
	char *str=(char*)p.getBuff(16);
	str="helloworld";
	printf("%x\n",&str);
	char *str2=(char*)p.getBuff(16);
	strcpy(str2,str);
	printf("%x\n",&str2);
	p.delBuff(str);
	std::cout<<str2<<std::endl;
	printf("%x\n",&str2);
	p.displaypool();
	return 0;
}
