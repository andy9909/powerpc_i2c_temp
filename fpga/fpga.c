#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/delay.h>

#include "fpga.h" 
#include "vpx3_prt_include.h"


MODULE_LICENSE("GPL");

#define MAJOR_NUM 249  //....

#define FPGA_TEMP_BASE_ADDR          0xf0000000             /*FPGA??Ҫд?Ļ???ַ  */
#define FPGA_TEMP_WRITE_SIZE         4*4            /*  4???¶ȼĴ??? */
#define FPGA_V_WRITE_SIZE         6*4            /*  6????ѹ?Ĵ???*/
#define FPGA_TEMP_ADDR   0x7c            /*0x7c - 0x8f  FPGA localbus?ϵĴ????¶? ֵ????????ÿ??32λ?Ĵ??? ֻ??ȡ????λ??Ϊ??һ??д?? ?��?.ֻ??16λ??????ֵ??ʱ?????ȶ??ٻ???ֵ*/
#define FPGA_V_ADDR  0xa0            /*0xa0 -0xbf */
#if 0 
struct {
    volatile int32_t * ptemp_addr,
             volatile int32_t * pv_addr,
}st_fpga_addr_base;/* ??Ҫд?????ڴ?λ?? */
#endif
/* ȫ????�� FPGA ӳ???ĵ?ַ */ 
volatile unsigned int * guipFpga;


static ssize_t fpga_info_read(struct file *, char *, size_t, loff_t*);
static ssize_t fpga_info_write(struct file *, const char *, size_t, loff_t*);
static int fpga_info_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
int static fpga_info_open(struct inode *pinode, struct file *pfile);

//..........file_operations...
struct file_operations fpga_fops =
{
      open: fpga_info_open,
      read: fpga_info_read,
      write: fpga_info_write,
      unlocked_ioctl: fpga_info_ioctl,
};
static char global_var[20] = {1,2,3,4,5,0};   //?globalvar?.......

int static fpga_info_open(struct inode *pinode, struct file *pfile)
{
    PRT_KFPGA_DEBUG(K_DEBUG_INFO,("===> %s\n", __func__));
    return 0;
}

static int __init fpga_init(void)
{
    int ret;

    //......
    register_chrdev(MAJOR_NUM, "fpga", &fpga_fops);
    guipFpga  = (unsigned int *)ioremap(FPGA_TEMP_BASE_ADDR, 0xff);
    PRT_KFPGA_DEBUG(K_DEBUG_INFO,("fpga register success"));
    return ret;
}

static void __exit fpga_exit(void)
{
    int ret;

    //......
    unregister_chrdev(MAJOR_NUM, "fpga");
    if (NULL != guipFpga)
    {
        iounmap(guipFpga);
    }

    PRT_KFPGA_DEBUG(K_DEBUG_INFO,("fpga un register success"));
}
/*****************************************************************************
  ?????? : fpga_info_ioctl
  ???????? : ????ʵ??FPGA ?Ĵ?????WORD ??д????
  ???????? : 
  ???????? :
  ????ֵ :
  ????:????
  ʱ??:2013-11-19
  ??ע:

 *****************************************************************************/

static int fpga_info_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    FPGA_CONTROL_CMD *pstFpgaCtl;
    unsigned short usRegAddr;
    volatile uint32_t * pmode_base_addr;/* д???ڴ??Ļ???ַ */
    unsigned short offset = 0;/* ָ???Ĵ??? ?ڵ?ǰ?ڴ???ƫ??ֵ*/
    unsigned int uiValue;
    int ret = 0;
    pstFpgaCtl = (FPGA_CONTROL_CMD*)&cmd;
    usRegAddr = pstFpgaCtl->usRegAddr;
    uiValue = arg;
    offset = usRegAddr/sizeof(unsigned int);
    int i=0;

    msleep(10);

    if (NULL == (char *)arg)
    {
        PRT_KFPGA_ERROR(("_user arg is NULL!\n"));
        return (-1);
    }

    if (NULL == guipFpga)
    {
        PRT_KFPGA_ERROR(("ioremap failed!\n"));
        return (-1);
    }
    pmode_base_addr = guipFpga;
#if 0
    PRT_KFPGA_DEBUG(K_DEBUG_INFO,("----> prink begin\n");
            for (i=0;i<40;i++)
            {
            PRT_KFPGA_DEBUG(K_DEBUG_INFO,("0x%x,", *(volatile unsigned int *)&guipFpga[i]);
                if (0 == i%4)
                {
                PRT_KFPGA_DEBUG(K_DEBUG_INFO,("\n");
                    }
                    }
                    PRT_KFPGA_DEBUG(K_DEBUG_INFO,("<---- prink end\n");
#endif


#if 0
                        /* ?ж? ???????? (?¶ȣ???ѹ?????�����??)?ڴ???дֵ??ƫ?ƻ???ַ */
                        switch (usRegAddr)
                        {
                        case FPGA_TEMP_ADDR:
                        pmode_base_addr =  st_fpga_addr_base.ptemp_addr;
                        offset = (usRegAddr - FPGA_TEMP_ADDR)/4;
                        break;
                        case FPGA_V_ADDR:
                        pmode_base_addr =  st_fpga_addr_base.pv_addr;
                        offset = (usRegAddr - FPGA_V_ADDR)/4;
                        break;
                        default :
                        break;
                        }
#endif
                        //PRT_KFPGA_DEBUG(K_DEBUG_INFO,("===> %s cmd [0x%x] pstFpgaCtl->ucCmd [0x%x] offset[%d]\n", __func__, cmd, pstFpgaCtl->ucCmd, offset);
       switch (pstFpgaCtl->ucCmd)
       {
           case FPGA_WRITE:
               {
                   /*?ڴ?ʵ??IOREMAPд,д????ֱ?Ӹ?ֵ*/

                   ret = __get_user(uiValue, (unsigned int *)arg);
                   pmode_base_addr[offset] = uiValue; 
                   //  	    PRT_KFPGA_DEBUG(K_DEBUG_INFO,("offset [%d] [uiValue][%x]\n", offset, uiValue ));
                   uiValue = 0;
                   break;
               }
           case FPGA_READ:
               {
                   /*?ڴ?ʵ??IOREMAP??ȡ?????Ĵ??????������??����?ַ????ַ??ֵ*/
                   // PRT_KFPGA_DEBUG(K_DEBUG_INFO,("ioctl FPGA_READ...\n");
                   uiValue =  (unsigned int)pmode_base_addr[offset];
                   //		    PRT_KFPGA_DEBUG(K_DEBUG_INFO,("===> %s Value [%x]\n", __func__, uiValue));
                   ret = __put_user(uiValue, (unsigned int *)arg);
                   //           PRT_KFPGA_DEBUG(K_DEBUG_INFO,("ret 0x%x uiValue [0x%x]\n", ret, uiValue));
                   break;
               }
           default:
               PRT_KFPGA_ERROR(("invild parme\n"));
                       break;
        }	
       PRT_KFPGA_DEBUG(K_DEBUG_INFO,("<=== %s\n", __func__));
}
/*****************************************************************************
  ?????? : fpga_info_read
  ???????? : ʵ??FPGA ?Ĵ????Ķ��???
  ???????? : 
  ???????? :
  ????ֵ :
  ????:????
  ʱ??:2013-11-19
  ??ע:

 *****************************************************************************/
static ssize_t fpga_info_read(struct file *filp, char *buf, size_t len, 
        loff_t *off)
{
    if (NULL == (char *)buf)
    {
        PRT_KFPGA_ERROR(("_user buf is NULL!\n"));
        return (-1);
    }

    if (copy_to_user(buf, &global_var, sizeof(global_var)))
    {
        PRT_KFPGA_ERROR(("copy to user failed!\n"));
        return - EFAULT;
    }
    PRT_KFPGA_DEBUG(K_DEBUG_INFO,("read string %u,%u\n",global_var[0],global_var[1]));
    return sizeof(global_var);
}
/*****************************************************************************
  ?????? : fpga_info_write
  ???????? : ʵ??FPGA ?Ĵ?????д????
  ???????? : 
  ???????? :
  ????ֵ :
  ????:????
  ʱ??:2013-11-19
  ??ע:

 *****************************************************************************/ 
static ssize_t fpga_info_write(struct file *filp, const char *buf, size_t len
        , loff_t  *off)
{
    if (NULL == (char *)buf)
    {
        PRT_KFPGA_ERROR(("_user buf is NULL!\n"));
        return (-1);
    }

    if (copy_from_user(&global_var, buf, sizeof(global_var)))
    {
        PRT_KFPGA_ERROR(("copy from user failed!\n"));
        return - EFAULT;
    }
    return sizeof(global_var);
}

module_init(fpga_init);
module_exit(fpga_exit);
