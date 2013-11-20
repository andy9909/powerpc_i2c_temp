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
#include <sys/statfs.h>

/*****************************************************************************
 func : ipAutoSet
 description : 根据单板的槽位信息自动配置serdes网络的ip/mask信息
 input : 网卡ID，IP地址，掩码，网关
 output : NA
 return : 0：success,1:fail
 作者:聂飞
 时间:2013-11-18
*****************************************************************************/
int   ipAutoSet(const   char   *ifname,   const   char   *ipaddr,const char *netmask,const char *gwip)
{
	struct   sockaddr_in   sin;
	struct ifreq ifr;
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
/*****************************************************************************
 函数名 : vpx3Ssd1GetSlotInfo
 函数描述 : 从字符设备(读取FPGA)获取SSD1单板获取单板槽位信息
 输入参数 : 
 输出参数 :
 返回值 :
 作者:聂飞
 时间:2013-11-18
  备注:
	GAP	奇偶校验   bit7
	CABIN0	设备ID编号 bit6
	CABIN1	设备ID编号 bit5
	GA4*	插槽编号   bit4
	GA3*	插槽编号   bit3
	GA2*	插槽编号   bit2
	GA1*	插槽编号   bit1
	GA0*	插槽编号   bit0
*****************************************************************************/
static unsigned char vpx3Ssd1GetSlotInfo(void)
{
#define FPGA_REG_NUM 40
#define FPAG_REG_SLOT_OFFSET 4
#define ERROR_CHAR -1
	int fd;
	unsigned char aucReg[FPGA_REG_NUM];

	fd = open("/dev/fpga_reg",O_RDWR,S_IRUSR | S_IWUSR);
	if(ERROR_CHAR != fd)
	{
		memset(aucReg, 0, FPGA_REG_NUM);
		read(fd,&aucReg,FPGA_REG_NUM);
		close(fd);
	}
	/*fpga reg 0x4 此处的reg地址为偏移后的地址，属于逻辑地址减去实际
	地址0xf0000000*/
	return aucReg[FPAG_REG_SLOT_OFFSET];
}

/*****************************************************************************
 函数名 : bmcAutoConfigEth
 函数描述 : 1:依据槽位自动完成IP地址设置
 输入参数 : 
 输出参数 :
 返回值 :
 作者:聂飞
 时间:2013-11-18
 备注:
	GAP	奇偶校验   bit7
	CABIN0	设备ID编号 bit6
	CABIN1	设备ID编号 bit5
	GA4*	插槽编号   bit4
	GA3*	插槽编号   bit3
	GA2*	插槽编号   bit2
	GA1*	插槽编号   bit1
	GA0*	插槽编号   bit0
*****************************************************************************/
static int bmcAutoConfigEth(void)
{
#define LENGTH_IPV4 16
#define SET_SUCCESS 0
/*第三个网络字节序.后字节偏移*/
#define IP_ADDR_4_OFFSET 10
	char *pNetMaskDefault = "255.255.255.0";
	char *pGateWayDefault = "192.168.1.1";
	char *pIpAddr="192.168.1.251";
	char aucIpAddr[LENGTH_IPV4 + 1];
	char ucSlotInfoReg = 0;
	char ucSlotId = 0;
	char ucDeviceId = 0;
	unsigned char ucValue = 0;
	int iRv = 0x1;

	/*设置默认IP地址*/
	memcpy(aucIpAddr,"192.168.1.251",LENGTH_IPV4);
	aucIpAddr[LENGTH_IPV4] = '\0';
	/*获取槽位ID和DEVICE_ID*/
	//ucSlotInfoReg = vpx3Ssd1GetSlotInfo();
	/*打桩验证*/
	ucSlotInfoReg = 0x74;
	ucDeviceId = (ucSlotInfoReg & 0x60)>> 0x5;
	ucSlotId = ucSlotInfoReg & 0x1f;
	ucValue = ((ucDeviceId << 0x5) + ucSlotId);
	/*依据规则直接修改IP地址0x0 2bit(deviceId) 5bit(slotId)*/
	if(ucValue / 100)
	{
		aucIpAddr[IP_ADDR_4_OFFSET] = ucValue / 100 + 48;
		ucValue = ucValue %100;
		aucIpAddr[IP_ADDR_4_OFFSET +1] = ucValue/10 + 48;
		ucValue = ucValue %10;
		aucIpAddr[IP_ADDR_4_OFFSET +2] = ucValue + 48;
	}
	else if(ucValue / 10)
	{
		aucIpAddr[IP_ADDR_4_OFFSET ] = (ucValue/10) + 48;
		ucValue = ucValue %10;
		aucIpAddr[IP_ADDR_4_OFFSET +1] = ucValue+48;
	}
	else
	{
		aucIpAddr[IP_ADDR_4_OFFSET] = ucValue + 48;
	}
	//int i = 0;
	//for(i = 0 ;i< 16; i++)
	{
	//	printf("aucIpAddr[%d]=%c\n",i,aucIpAddr[i]);
	}
	/*发起命令设置*/
	pIpAddr = aucIpAddr;
	printf("ip add = %s\n",pIpAddr);
        //ipAutoSet("em1","192.168.1.250","255.255.255.0","192.168.1.1");
	iRv = ipAutoSet("em1",pIpAddr,pNetMaskDefault,pGateWayDefault);
	if(SET_SUCCESS != iRv)
	{
		printf("auto set net ip addr fail,check card...\n");	
	}
}
/*****************************************************************************
 函数名 : nfsDiskInfoUpdate
 函数描述 : 更新NFS共享存储总容量和剩余可用容量
 输入参数 : 
 输出参数 :
 返回值 :
 作者:聂飞
 时间:2013-11-19
 备注:

*****************************************************************************/
static int nfsDiskInfoUpdate(void)
{
	struct statfs stDiskInfo;  
	unsigned long ulBlockSize = 0;//每个block里包含的字节数  
	unsigned long long  ullTotalSize = 0;//总的字节数，f_blocks为block的数目  
	unsigned long long ullAvailableSize = 0;
	unsigned long ulTotalMSize = 0; //共享磁盘总M数
	unsigned long ulAvailableMsize = 0; //共享磁盘剩余M数
	//unsigned long ulFreeSize = 0;
	
	statfs("/mnt", &stDiskInfo);  
	ulBlockSize = stDiskInfo.f_bsize;    
	ullTotalSize = ulBlockSize * stDiskInfo.f_blocks;   
	printf("Total_size = %llu B = %llu KB = %llu MB\n",   
	ullTotalSize, ullTotalSize>>10, ullTotalSize>>20);  

	//ulFreeSize = stDiskInfo.f_bfree * ulBlockSize; //剩余空间的大小  
	ullAvailableSize = stDiskInfo.f_bavail * ulBlockSize;   //可用空间大小  
	printf("Disk_available = %llu MB \n",ullAvailableSize>>20); 
	ulTotalMSize = ullTotalSize>> 20;
	ulAvailableMsize = ullAvailableSize >> 20;

	/*将信息刷新到FPAG中IOCTL控制寄存器读写*/
	

	return 0; 

}

/*****************************************************************************
 func : main
 description : 实现VPX3-SSD1单板的BMC功能
           1:依据槽位自动完成IP地址设置
           2: update nfs memory /mnt/ total_size and avaliable_size 
 input : 
 output :
 return :
 作者:聂飞
 时间:2013-11-18
*****************************************************************************/
void main()
{
#define SET_SUCCESS 0
	int iRv = 0x1;

	/*自动设置网卡IP地址上电配置一次即可*/
	iRv = bmcAutoConfigEth();
	if(SET_SUCCESS != iRv)
	{
		printf("bmc config eth \n");	
	}

    while(1)
	{
		sleep(5);
		/*更新磁盘容量信息*/
		(void)nfsDiskInfoUpdate();
		
	}
	return;
}

