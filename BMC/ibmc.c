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
#include <sys/ioctl.h>
/*housir: �ṹ�嶨�� */
/*FPGA��������ṹ��*/
typedef struct FPGA_CONTROL_CMD_ST
{
    unsigned short usRegAddr; /*�Ĵ�����ַ*/
    unsigned char ucReseved; /*Ԥ��*/
    unsigned char  ucCmd; /*cmd 0:��ȡ1:д�Ĵ���*/ 
}FPGA_CONTROL_CMD;

/*housir: �궨�� */
#define  FPGA_POSITION_REG           0x10 /*  */
#define  FPGA_TEMP0_REG           0x7C /*  */
#define  FPGA_TEMP1_REG           0x80 /*  */
#define  FPGA_TEMP2_REG           0x84 /*  */
#define  FPGA_TEMP3_REG           0x88 /*  */
#define  FPGA_VOLT0_REG           0xA0 /*  */
#define  FPGA_VOLT1_REG           0xA4 /*  */
#define  FPGA_VOLT2_REG           0xA8 /*  */
#define  FPGA_VOLT3_REG           0xAC /*  */
#define  FPGA_VOLT4_REG           0xB0 /*  */
#define  FPGA_VOLT5_REG           0xB4 /*  */
#define  FPGA_VOLT6_REG           0xB8 /*  */
#define  FPGA_TOTAL_DISK_INFO_REG           0xD0 /*  */
#define  FPGA_AVAILABLE_DISK_INFO_REG           0xD4 /*  */

#define FPGA_REG_READ       0     /*  */
#define FPGA_REG_WRITE       1     /*  */
/*housir: �� */
#define NEGATIVE       1            /*  ��ֵ*/
#define POSITIVE       0            /* ��ֵ */

#define TOTAL_SENSOES            10/*һ��10���������ɼ���*/
#define SENSOR_TEMP_TOTAL           4            /*4���¶Ȳɼ���  */
#define SENSOR_V_MAX_NUM       6            /*�������в�ĵ�ѹ�ĸ���  */

typedef struct lct2991_v_temp_data_tag{
    //    int16_t adc_code;/*housir: �¶Ȳɼ�����adc_code���ߵ�ѹ�ɼ�����code */
    unsigned char sign ;   /*housir: ����λ */ 
    int hvtemp ;     /*housir: �¶�/��ѹ������λ */
    int lvtemp ;     /*housir: �¶�/��ѹ��С��λ */    
}st_sensorinfo,*pst_sensorinfo;

/*housir: ȫ�ֱ��� */
st_sensorinfo stasensor_value [TOTAL_SENSOES] =
{
    {
        .sign     = POSITIVE,
        .hvtemp   = 0,
        .lvtemp   = 0,
    },
    0,
};
int gfd_fpga;/*housir: fpga �ַ��豸�������ļ�������*/
/*housir: �������� */
static int fpgaRegRead(unsigned int addr, unsigned int * Value);
static int fpgaRegWrite(unsigned int addr, unsigned int  Value);

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
    unsigned char aucReg[FPGA_REG_NUM];
    unsigned int uiRegValue;



    //	fd = open("/dev/fpga_reg",O_RDWR,S_IRUSR | S_IWUSR);
    if (-1 == fpgaRegRead(FPGA_POSITION_REG, &uiRegValue))
    {
        printf("fpgaRegRead\n");
    }

 //   printf("===> %s:uiRegValue [0x%x] \n", __func__, uiRegValue);
#if 0
    if(ERROR_CHAR != fd)
    {
        memset(aucReg, 0, FPGA_REG_NUM);
        read(fd,&aucReg,FPGA_REG_NUM);
        close(fd);
    }
#endif
    /*fpga reg 0x4 �˴���reg��ַΪƫ�ƺ�ĵ�ַ�������߼���ַ��ȥʵ��
      ��ַ0xf0000000*/
    return (uiRegValue&0xFF000000) >> 24;
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
    ucSlotInfoReg = vpx3Ssd1GetSlotInfo();

    //printf("===> %s:ucSlotInfoReg [%d] \n", __func__, ucSlotInfoReg);
    /*��׮��֤*/
    //	ucSlotInfoReg = 0x74;
    ucDeviceId = (ucSlotInfoReg & 0x60)>> 0x5;
    ucSlotId = ucSlotInfoReg & 0x1f;
    ucValue = ((ucDeviceId << 0x5) + ucSlotId);
    //printf("===> [%s] ucValue [0x%d]\n", __func__, ucValue);
    /*���ݹ���ֱ���޸�IP��ַ0x0 2bit(deviceId) 5bit(slotId)*/
    if(ucValue / 100)
    {
        aucIpAddr[IP_ADDR_4_OFFSET] = ucValue / 100 + 48;
        ucValue = ucValue %100;
        aucIpAddr[IP_ADDR_4_OFFSET +1] = ucValue/10 + 48;
        ucValue = ucValue %10;
        aucIpAddr[IP_ADDR_4_OFFSET +2] = ucValue + 48;
        aucIpAddr[IP_ADDR_4_OFFSET +3] = '0';
    }
    else if(ucValue / 10)
    {
        aucIpAddr[IP_ADDR_4_OFFSET ] = (ucValue/10) + 48;
        ucValue = ucValue %10;
        aucIpAddr[IP_ADDR_4_OFFSET +1] = ucValue+48;
        aucIpAddr[IP_ADDR_4_OFFSET +2] = '\0';
    }
    else
    {
        aucIpAddr[IP_ADDR_4_OFFSET] = ucValue + 48;
        aucIpAddr[IP_ADDR_4_OFFSET +1] = '0';
    }
    //int i = 0;
    //for(i = 0 ;i< 16; i++)
    {
        //	printf("aucIpAddr[%d]=%c\n",i,aucIpAddr[i]);
    }
    /*������������*/
    pIpAddr = aucIpAddr;
    //	printf("ip add = %s\n",pIpAddr);
    //ipAutoSet("em1","192.168.1.250","255.255.255.0","192.168.1.1");
    iRv = ipAutoSet("eth1",pIpAddr,pNetMaskDefault,pGateWayDefault);
    if(SET_SUCCESS != iRv)
    {
        printf("auto set net ip addr fail,check card...\n");	
    }
}
/**
 * @brief 
 *
 * @param addr
 * @param Value
 *
 * @return 
 */
static int fpgaRegRead(unsigned int addr, unsigned int * Value)
{
    int cmd;
    unsigned int uiRegValue;
    FPGA_CONTROL_CMD stcmd=
    {
        .usRegAddr = (unsigned short)addr,
        .ucReseved = 0,
        .ucCmd     = FPGA_REG_READ,
    };
    //    cmd = addr << 16  & 0xffffff00;
    //	fd = open("/dev/fpga_reg",O_RDWR,S_IRUSR | S_IWUSR);
    //printf("===> %s *(int *)&stcmd [0x%x], stcmd.usRegAddr [0x%x], stcmd.ucCmd [0x%x]\n", __func__
    //       ,*(int *)&stcmd, stcmd.usRegAddr, stcmd.ucCmd);
    // printf("sizeof(FPGA_CONTROL_CMD) [%d] &stcmd.usRegAddr [0x%x],  &stcmd.ucCmd [0x%x]\n", sizeof(stcmd), &stcmd.usRegAddr, &stcmd.ucCmd);

    //    if(ioctl(gfd_fpga, *(int *)&cmd, &uiRegValue) < 0)
    //    if(ioctl(gfd_fpga, 3, &uiRegValue) < 0)
    if(ioctl(gfd_fpga, *(int *)&stcmd, &uiRegValue) < 0)
    {
        uiRegValue = -1;
        *Value = uiRegValue;
        printf(" ioctl read error\n");
        return -1;
    }
    *Value = uiRegValue;

    return 0;
}

/**
 * @brief 
 *
 * @param addr
 * @param Value
 *
 * @return 
 */
static int fpgaRegWrite(unsigned int addr, unsigned int  Value)
{
    unsigned int uiRegValue = Value;
    FPGA_CONTROL_CMD stcmd=
    {
        .usRegAddr = (unsigned short)addr,
        .ucCmd     = FPGA_REG_WRITE,
    };

    //	fd = open("/dev/fpga_reg",O_RDWR,S_IRUSR | S_IWUSR);
    if(ioctl(gfd_fpga, *(int *)&stcmd, &uiRegValue) < 0)
    {
        printf(" ioctl write error\n");
        return -1;
    }

    return 0;
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
    unsigned long long ulBlockSize = 0;//ÿ��block��������ֽ���  
    unsigned long long  ullTotalSize = 0;//�ܵ��ֽ�����f_blocksΪblock����Ŀ  
    unsigned long long ullAvailableSize = 0;
    unsigned long long ulTotalMSize = 0; //���������M��
    unsigned long long ulAvailableMsize = 0; //�������ʣ��M��
    //unsigned long ulFreeSize = 0;

    statfs("/mnt", &stDiskInfo);  
    ulBlockSize = stDiskInfo.f_bsize;    
    ullTotalSize = ulBlockSize * stDiskInfo.f_blocks;   
//    printf("Total_size = %llu B = %llu KB = %llu MB\n",   
 //           ullTotalSize, ullTotalSize>>10, ullTotalSize>>20);  

    //ulFreeSize = stDiskInfo.f_bfree * ulBlockSize; //ʣ��ռ�Ĵ�С  
    ullAvailableSize = stDiskInfo.f_bavail * ulBlockSize;   //���ÿռ��С  
 //   printf("Disk_available = %llu MB \n",ullAvailableSize>>20); 
    ulTotalMSize = ullTotalSize>> 20;
    ulAvailableMsize = ullAvailableSize >> 20;

    /*����Ϣˢ�µ�FPAG��IOCTL���ƼĴ�����д*/
    if (-1 == fpgaRegWrite(FPGA_TOTAL_DISK_INFO_REG, ulTotalMSize))
    {
        printf("fpgaRegWrite FPGA_TOTAL_DISK_INFO_REG error!\n");
    }
    if (-1 == fpgaRegWrite(FPGA_AVAILABLE_DISK_INFO_REG, ulAvailableMsize))
    {
        printf("fpgaRegWrite FPGA_AVAILABLE_DISK_INFO_REG error!\n");
    }


    return 0; 

}
/**
 * @brief ���¶Ⱥ͵�ѹֵ д��FPGAָ���ڴ浥Ԫ��
 *
 * @return 
 */
static int TempVInfoUpdata(void)
{
    int index = 0;
    int n = 20;
    int dev_fd;
    int i=0;
    int vtemp_v = 0; 

    dev_fd = open("/dev/read_sensor",O_RDWR,S_IRUSR | S_IWUSR);
    if ( dev_fd == -1 ) 
    {
        printf("Cann't open file /dev/read_sensor\n");
        exit(1);
    }

    memset(stasensor_value, 0, sizeof(stasensor_value));
    read(dev_fd, stasensor_value, sizeof(stasensor_value));
    for( index=0;index<SENSOR_TEMP_TOTAL;index++)
    {
        vtemp_v = 0;
 //       printf("read v%d v%d tem is %d.%0.2d\n", 2*index+1, 2*index+2, stasensor_value[index].hvtemp , stasensor_value[index].lvtemp );
        vtemp_v =  stasensor_value[index].hvtemp*100 +  stasensor_value[index].lvtemp;
        if (NEGATIVE == stasensor_value[index].sign )
        {
            vtemp_v *= -1;
        }
        if (-1 == fpgaRegWrite(FPGA_TEMP0_REG + index*sizeof(unsigned int), vtemp_v))
        {
            printf("fpgaRegWrite FPGA_TEMP0_REG error!\n");
        }       

    }

    for( index=0;index<SENSOR_V_MAX_NUM;index++)
    {
        vtemp_v = 0;
 //       printf("read v%d V is %d.%0.2d\n", index+1, stasensor_value[ index + SENSOR_TEMP_TOTAL ].hvtemp ,  stasensor_value[ index + SENSOR_TEMP_TOTAL ].lvtemp );
        vtemp_v =  stasensor_value[index].hvtemp*100 +  stasensor_value[index].lvtemp;
        if (NEGATIVE == stasensor_value[index].sign )
        {
            vtemp_v *= -1;
        }

        if (-1 == fpgaRegWrite(FPGA_VOLT0_REG + index*sizeof(unsigned int), vtemp_v))
        {
            printf("fpgaRegWrite FPGA_TEMP0_REG error!\n");
        }      
    }

    close(dev_fd);

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

    gfd_fpga = open("/dev/fpga",O_RDWR,S_IRUSR | S_IWUSR);

    if(-1 == gfd_fpga)
    {
        printf("open failed !\n");
        return ;
    }

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
        (void)TempVInfoUpdata();


    }
    return;
}

