#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/delay.h>

#include "fpga.h" 
MODULE_LICENSE("GPL");
 
#define MAJOR_NUM 249  //....

#define FPGA_TEMP_BASE_ADDR          0xf0000000             /*FPGAÐèÒªÐ´µÄ»ùµØÖ·  */
#define FPGA_TEMP_WRITE_SIZE         4*4            /*  4¸öÎÂ¶È¼Ä´æÆ÷ */
#define FPGA_V_WRITE_SIZE         6*4            /*  6¸öµçÑ¹¼Ä´æÆ÷*/
#define FPGA_TEMP_ADDR   0x7c            /*0x7c - 0x8f  FPGA localbusÉÏµÄ´æ·ÅÎÂ¶È ÖµµÄÇøÓò£¬Ã¿¸ö32Î»¼Ä´æÆ÷ Ö»´æÈ¡ÆäµÍÎ»£¬ÎªÁËÒ»´ÎÐ´Èë ¶Á³ö.Ö»ÓÃ16Î»£¬Ôò¸³ÖµµÄÊ±ºòµÃÏÈ¶ÁÔÙ»ò¸³Öµ*/
#define FPGA_V_ADDR  0xa0            /*0xa0 -0xbf */
#if 0 
struct {
    volatile int32_t * ptemp_addr,
    volatile int32_t * pv_addr,
}st_fpga_addr_base;/* ÐèÒªÐ´ÈëµÄÄÚ´æÎ»ÖÃ */
#endif
/* È«²¿±äÁ¿ FPGA Ó³ÉäµÄµØÖ· */ 
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
static char global_var[20] = {1,2,3,4,5,0};   //“globalvar”.......

int static fpga_info_open(struct inode *pinode, struct file *pfile)
{
//    printk("===> %s\n", __func__);
    return 0;
}

static int __init fpga_init(void)
{
 int ret;
 
 //......
 register_chrdev(MAJOR_NUM, "fpga", &fpga_fops);
 guipFpga  = (unsigned int *)ioremap(FPGA_TEMP_BASE_ADDR, 0xff);
printk("fpga register success");
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

//     printk("fpga un register success");
}
/*****************************************************************************
 º¯ÊýÃû : fpga_info_ioctl
 º¯ÊýÃèÊö : ¿ØÖÆÊµÏÖFPGA ¼Ä´æÆ÷µÄWORD ¶ÁÐ´²Ù×÷
 ÊäÈë²ÎÊý : 
 Êä³ö²ÎÊý :
 ·µ»ØÖµ :
 ×÷Õß:Äô·É
 Ê±¼ä:2013-11-19
  ±¸×¢:

*****************************************************************************/

static int fpga_info_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	FPGA_CONTROL_CMD *pstFpgaCtl;
 	unsigned short usRegAddr;
    volatile int32_t * pmode_base_addr;/* Ð´ÈëÄÚ´æµÄ»ùµØÖ· */
    unsigned short offset = 0;/* Ö¸¶¨¼Ä´æÆ÷ ÔÚµ±Ç°ÄÚ´æµÄÆ«ÒÆÖµ*/
	unsigned int uiValue;
	int ret = 0;
	pstFpgaCtl = (FPGA_CONTROL_CMD*)&cmd;
	usRegAddr = pstFpgaCtl->usRegAddr;
	uiValue = arg;
    offset = usRegAddr/sizeof(unsigned int);
    int i=0;

	msleep(10);

    if (NULL == guipFpga)
    {
        printk("ioremap failed!\n");
        return -1;
    }
    pmode_base_addr = guipFpga;
#if 0
    printk("----> prink begin\n");
    for (i=0;i<40;i++)
    {
        printk("0x%x,", *(volatile unsigned int *)&guipFpga[i]);
        if (0 == i%4)
        {
            printk("\n");
        }
    }
    printk("<---- prink end\n");
#endif


#if 0
    /* ÅÐ¶Ï ÊÇÏòÄÄ×é (ÎÂ¶È£¬µçÑ¹£¬ÆäËû¡£¡£¡£)ÄÚ´æÖÐÐ´ÖµµÄÆ«ÒÆ»ùµØÖ· */
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
//    printk("===> %s cmd [0x%x] pstFpgaCtl->ucCmd [0x%x] offset[%d]\n", __func__, cmd, pstFpgaCtl->ucCmd, offset);
	switch (pstFpgaCtl->ucCmd)
	{
		case FPGA_WRITE:
		{
    	/*ÔÚ´ËÊµÏÖIOREMAPÐ´,Ð´²Ù×÷Ö±½Ó¸³Öµ*/

            pmode_base_addr[offset] = uiValue; 
			uiValue = 0;
			break;
		}
		case FPGA_READ:
		{
            /*ÔÚ´ËÊµÏÖIOREMAP¶ÁÈ¡µ¥¸ö¼Ä´æÆ÷²Ù×÷£¬¶Á²Ù×÷´«µØÖ·£¬µØÖ·¸³Öµ*/
           // printk("ioctl FPGA_READ...\n");
            uiValue =  (unsigned int)pmode_base_addr[offset];
			ret = __put_user(uiValue, (unsigned int *)arg);
  //           printk("ret 0x%x uiValue [0x%x]\n", ret, uiValue);
			break;
		}
		default:
			printk("invild parme\n");
			break;
	}	
//    printk("<=== %s\n", __func__);
}
/*****************************************************************************
 º¯ÊýÃû : fpga_info_read
 º¯ÊýÃèÊö : ÊµÏÖFPGA ¼Ä´æÆ÷µÄ¶Á²Ù×÷
 ÊäÈë²ÎÊý : 
 Êä³ö²ÎÊý :
 ·µ»ØÖµ :
 ×÷Õß:Äô·É
 Ê±¼ä:2013-11-19
  ±¸×¢:

*****************************************************************************/
static ssize_t fpga_info_read(struct file *filp, char *buf, size_t len, 
loff_t *off)
{
 if (copy_to_user(buf, &global_var, sizeof(global_var)))
 {
    return - EFAULT;
 }
//printk("read string %u,%u\n",global_var[0],global_var[1]);
 return sizeof(global_var);
}
/*****************************************************************************
 º¯ÊýÃû : fpga_info_write
 º¯ÊýÃèÊö : ÊµÏÖFPGA ¼Ä´æÆ÷µÄÐ´²Ù×÷
 ÊäÈë²ÎÊý : 
 Êä³ö²ÎÊý :
 ·µ»ØÖµ :
 ×÷Õß:Äô·É
 Ê±¼ä:2013-11-19
  ±¸×¢:

*****************************************************************************/ 
static ssize_t fpga_info_write(struct file *filp, const char *buf, size_t len
, loff_t
 *off)
{

 if (copy_from_user(&global_var, buf, sizeof(global_var)))
 {
    return - EFAULT;
 }
 return sizeof(global_var);
}
 
module_init(fpga_init);
module_exit(fpga_exit);
