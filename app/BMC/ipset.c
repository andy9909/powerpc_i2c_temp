#include   <stdio.h>
#include   <string.h>
#include   <sys/types.h>
#include   <sys/socket.h>
#include   <sys/ioctl.h>
#include   <netinet/in.h>
#include   <fcntl.h>
#include   <errno.h>
//#include   <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/route.h>
//ifname是网卡名，比如eth0,ipaddr是IP地址
int   ipset(const   char   *ifname,   const   char   *ipaddr,const char *netmask,const char *gwip)
{
    struct   sockaddr_in   sin;
    struct   ifreq   ifr;
    int   fd;

    bzero(&ifr,   sizeof(struct   ifreq));
    if   (ifname   ==   NULL)
        return   (-1);
    if   (ipaddr   ==   NULL)
        return   (-1);
    if(gwip == NULL)
        return(-1);
    fd   =   socket(AF_INET,   SOCK_DGRAM,   0);
    if   (fd   ==   -1)
    {
        perror( "Not   create   network   socket   connection\n ");
        return   (-1);
    }
    strncpy(ifr.ifr_name,   ifname,   IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ   -   1]   =   0;
    memset(&sin,   0,   sizeof(sin));
    sin.sin_family   =   AF_INET;
    sin.sin_addr.s_addr   =   inet_addr(ipaddr);
    memcpy(&ifr.ifr_addr,   &sin,   sizeof(sin));
    if   (ioctl(fd,   SIOCSIFADDR,   &ifr)   <   0)
    {
        perror( "Not   setup   interface\n ");
        return   (-1);
    }

    bzero(&ifr,   sizeof(struct   ifreq));
    strncpy(ifr.ifr_name,   ifname,   IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ   -   1]   =   0;
    memset(&sin,   0,   sizeof(sin));
    sin.sin_family   =   AF_INET;
    sin.sin_addr.s_addr   =   inet_addr(netmask);
    memcpy(&ifr.ifr_addr,   &sin,   sizeof(sin));
    if(ioctl(fd, SIOCSIFNETMASK, &ifr ) < 0)
    {
        perror("net mask ioctl error/n");
        return (-1);
    }
    struct rtentry rm;
    bzero(&rm,   sizeof(struct rtentry));
    rm.rt_dst.sa_family = AF_INET;
    rm.rt_gateway.sa_family = AF_INET;
    rm.rt_genmask.sa_family = AF_INET;
    memset(&sin,   0,   sizeof(sin));
    sin.sin_family   =   AF_INET;
    sin.sin_addr.s_addr   =   inet_addr(gwip);
    memcpy(&rm.rt_gateway, &sin,   sizeof(sin));
    rm.rt_dev = (char *)ifname;
    rm.rt_flags = RTF_UP | RTF_GATEWAY ;
    if(ioctl(fd, SIOCADDRT, &rm ) < 0)
    {
        perror("gateway ioctl error/n");
        return (-1);
    }
    ifr.ifr_flags   |=   IFF_UP   |   IFF_RUNNING;
    if (ioctl(fd,   SIOCSIFFLAGS,   &ifr)   <   0)
    {
        perror( "SIOCSIFFLAGS ");
        return   (-1);
    }
    return   (0);
}
void main()
{
	ipset("em1","192.168.1.250","255.255.255.0","192.168.1.1");
	return;
}

