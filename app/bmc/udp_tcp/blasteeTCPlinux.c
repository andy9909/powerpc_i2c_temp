/* blasteeTCPvx.c - TCP ethernet transfer benchmark for VxWorks */

/* Copyright 1986-2001 Wind River Systems, Inc. */

/*
   modification history
   --------------------
   04a,10may01,jgn  checkin for AE
   03c,20May99,gow  Split off from blastee.c
   03b,17May99,gow  Extended to zero-copy sockets
   03a,02Mar99,gow  Better documentation. Tested Linux as "blastee".
   02a,19dec96,lej  Fitted formatting to Wind River convention
   01b,15May95,ism  Added header
   01a,24Apr95,ism  Unknown Date of origin
   */

/*
   DESCRIPTION
   With this module, a VxWorks target ("blasteeTCP") receives
   blasts of TCP packets and reports network throughput on 10
   second intervals.

   SYNOPSIS
   blasteeTCP (port, size, bufsize, zbuf)

where:

port = port that "blasterTCP" should connect with
size = number of bytes per "blast"
bufsize = size of receive-buffer for blasteeTCP's BSD socket
(usually, size == bufsize)
zbuf = whether to use zero-copy sockets (1 = yes, 0 = no)

EXAMPLE
To start this test, issue the following VxWorks command on the
target:

sp blasteeTCP,5000,16000,16000

then issue the following UNIX commands on the host:

gcc -O -Wall -o blasterTCP blasterTCPux.c -lsocket   (solaris)
gcc -O -Wall -DLINUX -o blasterTCP blasterTCPux.c    (linux)
blasterTCP 10.255.255.8 5000 16000 16000  (use appropriate IP address)

Note: blasteeTCP should be started before blasterTCP because
blasterTCP needs a port to connect to.

To stop this test, call blasteeTCPQuit() in VxWorks or kill
blasterTCP on UNIX with a control-c.

The "blasterTCP"/"blasteeTCP" roles can also be reversed. That is,
the VxWorks target can be the "blasterTCP" and the UNIX host can
be the "blasteeTCP". In this case, issue the following UNIX
commands on the host:

gcc -O -Wall -o blasteeTCP blasteeTCPux.c -lsocket    (solaris)
gcc -O -Wall -DLINUX -o blasteeTCP blasteeTCPux.c     (linux)
blasteeTCP 5000 16000 16000

and issue the following VxWorks command on the target
(use appropriate IP address):

sp blasterTCP,"10.255.255.4",5000,16000,16000

To stop the test, call blasterTCPQuit() in VxWorks or kill
blasteeTCP on Unix with a control-c.

CAVEATS
Since this test loads the network heavily, the target and host
should have a dedicated ethernet link.

Be sure to compile VxWorks with zbuf support (INCLUDE_ZBUF_SOCK).
*/

//#include <vxworks.h>
//#include <ioLib.h>
//#include <logLib.h>
//#include "memLib.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
//#include <sockLib.h>
//#include "zbufSockLib.h"
//#include "zbufLib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sysLib.h>
//#include "tftpLib.h"
//#include <wdLib.h>
//#include <taskLib.h>

//static WDOG_ID blastWd;  /* for periodic throughput reports */
static int blastWd;  /* for periodic throughput reports */
static int blastNum;     /* number of bytes read per 10 second interval */
static char *buffer;            /* receive buffer */
static int sock;      /* server socket descriptor */
static int snew;      /* per-client socket descriptor */
static int tid;                 /* task ID for cleanup purposes */
//static int quitFlag;     /* flag for stopping test */
static int blasteeTCPquitFlag; 


/* forward declarations */

//static 
void blastRate (int);
//void blasteeTCPQuit(void);       /* global for shell accessibility */

/*****************************************************************
 *
 * blasteeTCP - Accepts blasts of TCP packets from blasterTCP
 *
 * This program accepts blasts of TCP packets from blasterTCP
 * and prints out the throughput every 10 seconds.
 *
 * blasteeTCP (port, size, bufsize, zbuf)
 *
 * where:
 *
 * port = TCP port to connect with on "blasterTCP"
 * size = number of bytes per "blast"
 * bufsize = size of receive-buffer for blasteeTCP's BSD socket
 * zbuf = whether to use zero-copy sockets (1 = yes, 0 = no)
 *
 * RETURNS:  N/A
 */

void blasteeTCP (int port, int size, int blen, int zbuf)
{
    struct sockaddr_in		serverAddr, clientAddr;
    int                 len;    /* sizeof(clientAddr) */
    int                 nrecv;  /* number of bytes received */
    printf("==> %s port [%d], size [%d]. blen [%d], zbuf [%d]\n", __func__, port, size, blen, zbuf);
    //   ZBUF_ID             zid;    /* zero-copy buffer ID */

    //    tid = taskIdSelf();
    if ((buffer = (char *) malloc (size+1)) == NULL)
    {
        printf ("cannot allocate buffer of size %d\n", size);
        return;
    }

    /* setup watchdog to periodically report network throughput */
#if 0
    if ((blastWd = wdCreate ()) == NULL)
    {
        printf ("cannot create blast watchdog\n");
        free (buffer);
        return;
    }
    wdStart (blastWd, 100*1, (FUNCPTR) blastRate, 0);
#endif

    alarm(10);
    printf("wd started\n");
    /* setup BSD socket to read blasts from */

    bzero ((char *) &serverAddr, sizeof (serverAddr));
    bzero ((char *) &clientAddr, sizeof (clientAddr));

    if ((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf ("cannot open socket\n");
        //        wdDelete (blastWd);
        free (buffer);
        return;
    }

    serverAddr.sin_family       = AF_INET;
    serverAddr.sin_port = htons (port);
    //	serverAddr.sin_addr.s_addr	= inet_addr ("10.0.212.171");     // add

    if (bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)//将一个本地的网络运输层和套接字联系起来
    {
        printf ("bind error\n");
        close (sock);
        //        wdDelete (blastWd);
        free (buffer);
        exit(0);
        return;
    }

    if (listen (sock, 5) < 0) //通知斜进程准备接受套接字上的连接请求
    {
        printf ("listen failed\n");
        close (sock);
        //        wdDelete (blastWd);
        free (buffer);
        exit(0);
        return;
    }

    len = sizeof (clientAddr);

    while ((snew = accept(sock, (struct sockaddr *)&clientAddr, &len)) == -1)//等待连接请求
        ;

    printf("accepted\n");
    //return;
    blastNum = 0;
    blasteeTCPquitFlag = 0;

    if (setsockopt (snew, SOL_SOCKET, SO_RCVBUF,
                (char *) &blen, sizeof (blen)) < 0)  //插入进程请求的套接字和协议选项
    {
        printf ("setsockopt SO_RCVBUF failed\n");
        close (sock);
        close (snew);
        //        wdDelete (blastWd); //删除看门狗
        free (buffer);
        return;
    }

    /* loop that reads TCP blasts */

    for (;;)
    {
        //  	printf("in for \n");
        if (blasteeTCPquitFlag == 1)
            break;
        /*       if (zbuf)
                 {
                 nrecv = size;
                 if ((zid = zbufSockRecv(snew, 0, &nrecv)) == NULL)
                 nrecv = -1;
                 else
                 zbufDelete(zid);
                 } */
        //        else
        nrecv = read (snew, buffer, size);//从filp指定的字符设备中读取数据
        buffer[size] = 0;
        //printf(" ========================================buffer =\n %s\n", buffer);
        if (nrecv < 0)
        {
            printf ("blasteeTCP read error\n");
            break;
        }
        //    printf("received %d\n", nrecv);
        blastNum += nrecv;
    }

    /* cleanup */

    //    wdDelete (blastWd);
    close (sock);
    close (snew);
    free (buffer);
    printf ("--------------------blasteeTCP end.\n");
//    sleep(5);
}


/* watchdog handler. reports network throughput */

void blastRate (int signal_num)
{
    if (blastNum > 0)
    {
        printf ("%d bytes/sec\n", blastNum/10 );
        blastNum = 0;
    }
    else
    {
        printf ("No bytes read in the last 10 seconds.\n");/*cancle by jyl*/
    }
    alarm(10);
    //    wdStart (blastWd, 1000*10 , (FUNCPTR) blastRate, 0);//开始看门狗定时器
}
#if 0
/* make blasteeTCP stop */

void blasteeTCPQuit(void)
{
    blasteeTCPquitFlag = 1;
    taskDelay(60);               /* try to end gracefully */
    if (taskIdVerify(tid) == OK) /* blasteeTCP still trying to read() */
    {
        close (sock);
        close (snew);
        wdDelete (blastWd);
        free (buffer);
        taskDelete (tid);
        printf ("blasteeTCP forced stop.\n");
    }
}

#endif
#include	<stdlib.h>


static unsigned char str2decnum(unsigned char c)
{                        
    if(c >= '0' && c <= '9')
        return c - '0';                                                         
#if 0
    if(c >= 'a' && c <= 'f')
        return c - 'a' + 10; 
    if(c >= 'A' && c <= 'F')
        return c - 'A' + 10; 
#endif
    return 0; /* foo */
}    

/**
 * @brief  str转换成10进制
 *
 * @param str
 *
 * @return 
 */
static int str2dec(unsigned char *str)
{                     
    int value = 0;    
    while (*str) {    
        value = value * 10;                  
        value += str2decnum(*str++);
    }                 
                      
    return value;     
}       /* -----  end of function str2hex  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int main ( int argc, char *argv[] )
{
    signal(SIGALRM, blastRate);

    blasteeTCP(str2dec(argv[1]), str2dec(argv[2]), str2dec(argv[3]), str2dec(argv[4]));

    return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */

