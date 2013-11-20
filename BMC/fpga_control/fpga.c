#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include "fpga.h" 
MODULE_LICENSE("GPL");
 
#define MAJOR_NUM 249  //....
 
static ssize_t fpga_info_read(struct file *, char *, size_t, loff_t*);
static ssize_t fpga_info_write(struct file *, const char *, size_t, loff_t*);
static ssize_t fpga_info_ioctl(struct inode *inodep,struct file *filp, unsigned int cmd, unsigned long arg);

 
//..........file_operations...
struct file_operations fpga_fops =
{
  read: fpga_info_read,
  write: fpga_info_write,
  unlocked_ioctl: fpga_info_ioctl,
};
static char global_var[20] = {1,2,3,4,5,0};   //�globalvar�.......
static int __init fpga_init(void)
{
 int ret;
 
 //......
 register_chrdev(MAJOR_NUM, "fpga", &fpga_fops);
printk("fpga register success");
 return ret;
}
 
static void __exit fpga_exit(void)
{
 int ret;
 
 //......
  unregister_chrdev(MAJOR_NUM, "fpga");
printk("fpga un register success");
}
/*****************************************************************************
 ������ : fpga_info_ioctl
 �������� : ����ʵ��FPGA �Ĵ�����WORD ��д����
 ������� : 
 ������� :
 ����ֵ :
 ����:����
 ʱ��:2013-11-19
  ��ע:

*****************************************************************************/

static ssize_t fpga_info_ioctl(struct inode *inodep,struct file *filp, unsigned int cmd, unsigned long arg)
{
	FPGA_CONTROL_CMD *pstFpgaCtl;
 	unsigned short usRegAddr;
	unsigned int uiValue;
	int ret = 0;
	pstFpgaCtl = (FPGA_CONTROL_CMD*)&cmd;
	usRegAddr = pstFpgaCtl->usRegAddr;
	uiValue = arg;
	switch (pstFpgaCtl->ucCmd)
	{
		case FPGA_WRITE:
		{
	/*�ڴ�ʵ��IOREMAPд,д����ֱ�Ӹ�ֵ*/
			uiValue = 0;
			break;
		}
		case FPGA_READ:
		{
/*�ڴ�ʵ��IOREMAP��ȡ�����Ĵ�������������������ַ����ַ��ֵ*/
			ret = __put_user(uiValue, (unsigned int *)arg);
			break;
		}
		default:
			printk("invild parme\n");
			break;
	}	
}
/*****************************************************************************
 ������ : fpga_info_read
 �������� : ʵ��FPGA �Ĵ����Ķ�����
 ������� : 
 ������� :
 ����ֵ :
 ����:����
 ʱ��:2013-11-19
  ��ע:

*****************************************************************************/
static ssize_t fpga_info_read(struct file *filp, char *buf, size_t len, 
loff_t *off)
{
 if (copy_to_user(buf, &global_var, sizeof(global_var)))
 {
    return - EFAULT;
 }
printk("read string %u,%u\n",global_var[0],global_var[1]);
 return sizeof(global_var);
}
/*****************************************************************************
 ������ : fpga_info_write
 �������� : ʵ��FPGA �Ĵ�����д����
 ������� : 
 ������� :
 ����ֵ :
 ����:����
 ʱ��:2013-11-19
  ��ע:

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
