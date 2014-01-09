/*
 * =====================================================================================
 *       Copyright (c), 2013-2020, Prophet C&S.
 *       Filename:  main.c
 *
 *    Description:  产生一个G的文件,用以测试速度
 *         Others:
 *   
 *        Version:  1.0
 *        Date:  Thursday, November 21, 2013 06:48:10 HKT
 *       Revision:  none
 *       Compiler:  powerpc-gcc
 *
 *         Author:  housir , houwentaoff@gmail.com
 *   Organization:  Prophet
 *        History:   Created by housir
 *
 * =====================================================================================
 */


#define _GNU_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include "string.h"
#define IO_DIRET

void main()
{
#define SIZE 4096*100
#define LOOP_TIMES 250
        char *p1;
	char *q = 0;
	char *buf;
	char fd;
	time_t lt,now;
	int i = LOOP_TIMES;
	int total = 0;
	long long uiRet;
#ifndef IO_DIRET
	printf("now not  io_direct \n");
	p1 = (char *)malloc(SIZE +4096);

	if(NULL == p1)
	{
		printf("errr malloc \n");
	}
	
	memset(p1,0x35,SIZE+4096);
	fd = open("hello", O_CREAT | O_RDWR, S_IRWXU);
#else
	printf("io_direct \n");
	uiRet = posix_memalign((void **)&buf, 4096, SIZE);
	memset(buf,0x36,SIZE);
	if(uiRet)
	{
		printf("posix_memalign error %x",uiRet);
	}
	fd = open("hello",O_DIRECT | O_CREAT | O_RDWR, S_IRWXU);
#endif
	if(fd < 0)
	{
		printf("open faill\n");
		return;
	}
	time(&lt);
	printf("wrie : begin %s",ctime(&lt));
	while(	i )
	{
#ifndef IO_DIRET	
	q = p1;
	uiRet = write(fd,q,SIZE);	
#else
	uiRet = write(fd,buf,SIZE);
#endif
		i--;	
		if(uiRet < 0)
		{
			printf("write fail uiRet = %x\n",uiRet);
		}
//		printf("write ok i = %x\n",i);
	}
	        time(&now);
        printf("write :end %s\n",ctime(&now));
	close(fd);
	printf("read :begin %s\n",ctime(&now));
	
#ifndef IO_DIRET
	fd = open("hello", O_CREAT | O_RDWR, S_IRWXU);
	if(fd < 0)
	{
		printf("open again fali\n");
	}		
		//printf("not io_direct read\n");
		while((i = read(fd,q,SIZE)) > 0)
		{
			total += i;
			//////printf("now i = 0x%x",i);

		}
		printf("not io_direct read total = 0x%x\n",total);
#else
	fd = open("hello",O_DIRECT | O_CREAT | O_RDWR, S_IRWXU);
        if(fd == -1)
        {
                printf("open again faill\n");
        }

		//printf("io_direct read\n");
		while((i = read(fd,buf,SIZE)) > 0)
		{
                        total += i;

		}
		 printf("io_direct read total = 0x%x\n",total);
#endif
		if(uiRet< 0)
		{
			printf(" read fail %x\n",uiRet);
			
		}
	time(&now);
        printf("read :end %s\n",ctime(&now));
	close(fd);
#ifndef IO_DIRET	
	free(p1);
#else
	free(buf);
#endif

}
