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
 description : ���ݵ���Ĳ�λ��Ϣ�Զ�����serdes�����ip/mask��Ϣ
 input : ����ID��IP��ַ�����룬����
 output : NA
 return : 0��success,1:fail
 ����:����
 ʱ��:2013-11-18
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
 ������ : vpx3Ssd1GetSlotInfo
 �������� : ���ַ��豸(��ȡFPGA)��ȡSSD1�����ȡ�����λ��Ϣ
 ������� : 
 ������� :
 ����ֵ :
 ����:����
 ʱ��:2013-11-18
  ��ע:
	GAP	��żУ��   bit7
	CABIN0	�豸ID��� bit6
	CABIN1	�豸ID��� bit5
	GA4*	��۱��   bit4
	GA3*	��۱��   bit3
	GA2*	��۱��   bit2
	GA1*	��۱��   bit1
	GA0*	��۱��   bit0
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
	/*fpga reg 0x4 �˴���reg��ַΪƫ�ƺ�ĵ�ַ�������߼���ַ��ȥʵ��
	��ַ0xf0000000*/
	return aucReg[FPAG_REG_SLOT_OFFSET];
}

/*****************************************************************************
 ������ : bmcAutoConfigEth
 �������� : 1:���ݲ�λ�Զ����IP��ַ����
 ������� : 
 ������� :
 ����ֵ :
 ����:����
 ʱ��:2013-11-18
 ��ע:
	GAP	��żУ��   bit7
	CABIN0	�豸ID��� bit6
	CABIN1	�豸ID��� bit5
	GA4*	��۱��   bit4
	GA3*	��۱��   bit3
	GA2*	��۱��   bit2
	GA1*	��۱��   bit1
	GA0*	��۱��   bit0
*****************************************************************************/
static int bmcAutoConfigEth(void)
{
#define LENGTH_IPV4 16
#define SET_SUCCESS 0
/*�����������ֽ���.���ֽ�ƫ��*/
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

	/*����Ĭ��IP��ַ*/
	memcpy(aucIpAddr,"192.168.1.251",LENGTH_IPV4);
	aucIpAddr[LENGTH_IPV4] = '\0';
	/*��ȡ��λID��DEVICE_ID*/
	//ucSlotInfoReg = vpx3Ssd1GetSlotInfo();
	/*��׮��֤*/
	ucSlotInfoReg = 0x74;
	ucDeviceId = (ucSlotInfoReg & 0x60)>> 0x5;
	ucSlotId = ucSlotInfoReg & 0x1f;
	ucValue = ((ucDeviceId << 0x5) + ucSlotId);
	/*���ݹ���ֱ���޸�IP��ַ0x0 2bit(deviceId) 5bit(slotId)*/
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
	/*������������*/
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
 ������ : nfsDiskInfoUpdate
 �������� : ����NFS����洢��������ʣ���������
 ������� : 
 ������� :
 ����ֵ :
 ����:����
 ʱ��:2013-11-19
 ��ע:

*****************************************************************************/
static int nfsDiskInfoUpdate(void)
{
	struct statfs stDiskInfo;  
	unsigned long ulBlockSize = 0;//ÿ��block��������ֽ���  
	unsigned long long  ullTotalSize = 0;//�ܵ��ֽ�����f_blocksΪblock����Ŀ  
	unsigned long long ullAvailableSize = 0;
	unsigned long ulTotalMSize = 0; //���������M��
	unsigned long ulAvailableMsize = 0; //�������ʣ��M��
	//unsigned long ulFreeSize = 0;
	
	statfs("/mnt", &stDiskInfo);  
	ulBlockSize = stDiskInfo.f_bsize;    
	ullTotalSize = ulBlockSize * stDiskInfo.f_blocks;   
	printf("Total_size = %llu B = %llu KB = %llu MB\n",   
	ullTotalSize, ullTotalSize>>10, ullTotalSize>>20);  

	//ulFreeSize = stDiskInfo.f_bfree * ulBlockSize; //ʣ��ռ�Ĵ�С  
	ullAvailableSize = stDiskInfo.f_bavail * ulBlockSize;   //���ÿռ��С  
	printf("Disk_available = %llu MB \n",ullAvailableSize>>20); 
	ulTotalMSize = ullTotalSize>> 20;
	ulAvailableMsize = ullAvailableSize >> 20;

	/*����Ϣˢ�µ�FPAG��IOCTL���ƼĴ�����д*/
	

	return 0; 

}

/*****************************************************************************
 func : main
 description : ʵ��VPX3-SSD1�����BMC����
           1:���ݲ�λ�Զ����IP��ַ����
           2: update nfs memory /mnt/ total_size and avaliable_size 
 input : 
 output :
 return :
 ����:����
 ʱ��:2013-11-18
*****************************************************************************/
void main()
{
#define SET_SUCCESS 0
	int iRv = 0x1;

	/*�Զ���������IP��ַ�ϵ�����һ�μ���*/
	iRv = bmcAutoConfigEth();
	if(SET_SUCCESS != iRv)
	{
		printf("bmc config eth \n");	
	}

    while(1)
	{
		sleep(5);
		/*���´���������Ϣ*/
		(void)nfsDiskInfoUpdate();
		
	}
	return;
}

