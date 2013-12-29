/* blasteeUDPvx.c - UDP ethernet transfer benchmark for VxWorks */

/* Copyright 1986-2001 Wind River Systems, Inc. */

/*               
                 modification history
                 --------------------
                 04a,10may01,jgn  checkin for AE
                 01a,18May99,gow  Created from blastee.c
                 */                               

/*
   DESCRIPTION
   With this module, a VxWorks target ("blasteeUDP") receives
   blasts of UDP packets and reports network throughput on 10
   second intervals.

   SYNOPSIS
   blasteeUDP (port, size, bufsize, zbuf)

where:

port = UDP port for receiving packets 接收数据包的UDP口
size = number of bytes per "blast"    
bufsize = size of receive-buffer for blasteeUDP's BSD socket
(usually, size == bufsize)
zbuf = whether to use zero-copy sockets (1 = yes, 0 = no)

EXAMPLE
To start this test, issue the following VxWorks command on the
target:

sp blasteeUDP,5000,1000,1000

then issue the following UNIX commands on the host:

gcc -O -Wall -o blasterUDP blasterUDPux.c -lsocket   (solaris)
gcc -O -Wall -DLINUX -o blasterUDP blasterUDPux.c    (linux)
blasterUDP 10.255.255.8 5000 1000 1000  (use appropriate IP address)

Note: blasteeUDP should be started before blasterUDP because
blasterUDP needs a port to send UDP packets to.

To stop this test, call blasteeUDPQuit() in VxWorks or kill
blasterUDP on UNIX with a control-c.

The "blasterUDP"/"blasteeUDP" roles can also be reversed. That is,
the VxWorks target can be the "blasterUDP" and the UNIX host can
be the "blasteeUDP". In this case, issue the following UNIX
commands on the host:

gcc -O -Wall -o blasteeUDP blasteeUDPux.c -lsocket    (solaris)
gcc -O -Wall -DLINUX -o blasteeUDP blasteeUDPux.c     (linux)
blasteeUDP 5000 1000 1000

and issue the following VxWorks command on the target
(use appropriate IP address):

sp blasterUDP,"10.255.255.4",5000,1000,1000

To stop the test, call blasterUDPQuit() in VxWorks or kill
blasteeUDP on Unix with a control-c.

CAVEATS
Since this test loads the network heavily, the target and host
should have a dedicated ethernet link.

Be sure to compile VxWorks with zbuf support (INCLUDE_ZBUF_SOCK).

The bufsize parameter is disabled for VxWorks. Changing UDP socket
buffer size with setsockopt() in VxWorks somehow breaks the socket.
*/

//#include <vxworks.h>
//#include <ioLib.h>
//#include <logLib.h>
//#include "memLib.h"
#include <sys/socket.h>
#include <netinet/in.h>
//#include <sockLib.h>
//#include "zbufSockLib.h"
//#include "zbufLib.h"
#include <stdio.h>		
#include <stdlib.h>
#include <string.h>
#include <signal.h>

//#include <sysLib.h>
//#include "tftpLib.h"
//#include <wdLib.h>
//#include <taskLib.h>
#include <errno.h>

static int tid;                 /* task ID for cleanup purposes */
//static WDOG_ID blastWd;         /* for periodic throughput reports */
static char *buffer;  /* receive buffer */
static int sock;      /* receiving socket descriptor */
static int blastNum;   /* number of bytes read per 10 second interval 每十秒所读到的字节数*/
//static int quitFlag;   /* set to 1 to quit blasteeUDP gracefully */
static int blasteeUDPquitFlag;


/* forward declarations */

static void blastRate (int );
//void blasteeUDPQuit(void);       /* global for shell accessibility */


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


/*****************************************************************
 *
 * blasteeUDP - Accepts blasts of UDP packets from blasterUDP
 * 
 * This function accepts blasts of UDP packets from blasterUDP
 * and reports the throughput every 10 seconds.
 * 
 * blasteeUDP (port, size, bufsize, zbuf)
 * 
 * where:
 * 
 * port = UDP port to connect with on "blasterUDP"？？？？？？？
 * size = number of bytes per "blast"
 * bufsize = size of receive-buffer for blasteeUDP's BSD socket
 * zbuf = whether to use zero-copy sockets (1 = yes, 0 = no)
 * 
 * RETURNS:  N/A
 */

void blasteeUDP (int port, int size, int blen, int zbuf)
{
    struct sockaddr_in		serverAddr, clientAddr;
    int                 nrecv;  /* number of bytes received */
    int sockAddrSize = sizeof(struct sockaddr);
    //   ZBUF_ID             zid;    /* zero-copy buffer ID */
    
    printf("==> %s port [%d], size [%d]. blen [%d], zbuf [%d]\n", __func__, port, size, blen, zbuf);


//    tid = taskIdSelf();
    buffer = (char *) malloc (size);
    if (buffer == NULL)
    {
        printf ("cannot allocate buffer of size %d\n", size);
        return;
    }

#if 0
    /* setup watchdog to periodically report network throughput
    */
    if ((blastWd = wdCreate ()) == NULL)
    {
        printf ("cannot create blast watchdog\n");
        free (buffer);   //释放掉？？
        return;
    }
    wdStart (blastWd, sysClkRateGet ()*10, (FUNCPTR) blastRate, 0);
#endif

    alarm(10);

    if ((sock = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf ("cannot open socket\n");
//        wdDelete (blastWd);
        free (buffer);
        return;
    }
    //??????????????????????????????????????????????
    serverAddr.sin_family	= AF_INET;
    serverAddr.sin_port	= htons (port);
    serverAddr.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */
    bzero((char *)&serverAddr.sin_zero, 8);  /* zero rest of struct */
    //?????????????????????????????????????????????????
    if (bind (sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)//将一个本地的网络运输层地址和套接字联系起来
    {
        printf ("bind error\n");
        close (sock);
//        wdDelete (blastWd);
        free (buffer);
        return;
    }

    /* setting buffer size breaks UDP sockets for some reason.  This
     * happens on Linux too (but not Solaris). Hence setsockopt() is
     * commented till further notice.
     */

    if (setsockopt (sock, SOL_SOCKET, SO_RCVBUF,
                (char *) &blen, sizeof (blen)) < 0)//插入进程请求的套接字和协议选项
    {
        printf ("setsockopt SO_RCVBUF failed\n");
        close (sock);
//        wdDelete (blastWd);
        free (buffer);
        exit (1);
    }


    blastNum = 0;
    blasteeUDPquitFlag = 0;

    /* loop that reads UDP blasts */
    int ct =0;
    for (;;)
    {
        if (blasteeUDPquitFlag)
            break;
        /*       if (zbuf)
                 {
                 nrecv = size;
                 if ((zid = zbufSockRecvfrom(sock, 0, &nrecv,
                 (struct sockaddr *)&clientAddr,
                 (int *)&sockAddrSize)) == NULL)
                 nrecv = -1;
                 else
                 zbufDelete(zid);
                 }
                 else  */
        nrecv = recvfrom (sock, (char *)buffer, size, 0, 
                (struct sockaddr *)&clientAddr,
                (int *)&sockAddrSize);//接收远程主机经指定的socket传来的数据
        //        printf("setsockopt receive\n");/*LYT*/
        if (nrecv < 0)
        {
            printf ("blasteeUDP read error %d\n", errno);//errno??????????????????????????????????
            break;
        }
        //        if(++ct%1000==0)
        //        printf ("blasteeUDP received %d--------------\n", nrecv);        
        blastNum += nrecv;
    }
    //??????????????????????????????????????????????????????????????
    close (sock);
//    wdDelete (blastWd);
    free (buffer);
    printf ("blasteeUDP end.\n");
}

/* watchdog handler. reports network throughput */

static void blastRate (int signal_num)
{
    if (blastNum > 0)
    {
        printf ("%d bytes/sec\n", blastNum/10);
        blastNum = 0;
    }
    else
    {
        printf ("No bytes read in the last 10 seconds.\n");
    }
    alarm(10);

//    wdStart (blastWd, sysClkRateGet ()*10, (FUNCPTR) blastRate, 0);
}


#include	<stdlib.h>

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int main ( int argc, char *argv[] )
{
    signal(SIGALRM, blastRate);

    blasteeUDP(str2dec(argv[1]), str2dec(argv[2]), str2dec(argv[3]), str2dec(argv[4]));

    return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
/* make blasteeUDP stop */
#if 0
void blasteeUDPQuit(void)
{
    blasteeUDPquitFlag = 1;
    taskDelay(60);               /* try to end gracefully */
    if (taskIdVerify(tid) == OK) /* blasteeUDP still trying to receive */
    {
        close (sock);
        wdDelete (blastWd);
        free (buffer);
        taskDelete (tid);
        printf ("blasteeUDP forced stop.\n");
    }
}
#endif
