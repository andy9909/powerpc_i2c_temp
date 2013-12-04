/*
 * =====================================================================================
 *       Copyright (c), 2013-2020, Prophet C&S.
 *       Filename:  i2c_ltc2991.c
 *
 *    Description:  i2c驱动
 *         Others:
 *   
 *        Version:  1.0
 *        Date:  Sunday, November 10, 2013 01:57:08 HKT
 *       Revision:  none
 *       Compiler:  powerpc-gcc
 *
 *         Author:  housir , houwentaoff@gmail.com
 *   Organization:  prophet
 *        History:   Created by housir
 *
 * =====================================================================================
 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>  
#include <linux/slab.h>   
#include <linux/fs.h>       
#include <linux/errno.h>   
#include <linux/types.h>   
#include <linux/mm.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/device.h>

#include "i2c_ltc2991.h"
#include "vpx3_prt_include.h"

#define    LCT2991_MINOR  222 /*  */ 
#define LCT2991_MAJOR    250            /*  */

#if 0
#define FPGA_TEMP_ADDR   0x7c            /*0x7c - 0x8f  FPGA localbus上的存放温度 值的区域，每个32位寄存器 只存取其低位，为了一次写入 读出.只用16位，则赋值的时候得先读再或赋值*/
#define FPGA_V_ADDR  0xa0            /*0xa0 -0xbf */
#endif

/*housir: 驱动调试等级 */
unsigned long KDebugLevel = K_DEBUG_INFO;

static int lct2991_major = LCT2991_MAJOR;
static int number_of_devices = 1;


enum chips { lct2991_temp, lct2991_v};
/*
 * Addresses to scan
 * LCT2991 have address 0x4d or 0x4b.
 */

static const unsigned short normal_i2c[] = { 0x4d, 0x4b, I2C_CLIENT_END };
I2C_CLIENT_INSMOD;


/*
 * Driver data (common to all clients)
 */

static const struct i2c_device_id lct2991_idtable[] = {
	{ "lct2991", lct2991_temp },
	{ "lct2991", lct2991_v },
	{ }
};
MODULE_DEVICE_TABLE(i2c, lct2991_idtable);/*housir:  MODULE_DEVICE_TABLE宏是用来生成i2c_device_id。在legacy方式中i2c_client是自己创建的，而此处的i2c_client如何得到？实际上是在 i2c_register_board_info函数注册i2c_board_info过程中构建的 */

static int __devinit lct2991_probe(struct i2c_client *client,
                      const struct i2c_device_id *id);
static int __devexit lct2991_remove(struct i2c_client *client);


/*
 * Driver data (common to all clients)
 */
static struct i2c_driver lct2991_i2c_driver = {
//	.class		= I2C_CLASS_HWMON,
	.driver = {
		.name	= "lct2991",
//        .owner = THIS_MODULE,
	},
	.probe		= lct2991_probe,
	.remove		= lct2991_remove,
	.id_table	= lct2991_idtable,
//	.detect		= lct2991_detect,/*housir:  注销呢?*/
	.address_list	= normal_i2c,
//    .command    = NULL,/*housir: optional, deprecated */
};
/*
 * Client data (each client gets its own)
 */

struct lct2991_data {
    struct i2c_client *client;
    int users;// how many users using the driver
}*pstlct2991_data;

static struct i2c_client *new_client = NULL;

static struct cdev i2c_lct2991_dev;

static dev_t devno = 0;
#if 0
static struct i2c_board_info i2c_devs0[] __initdata = {                                                                                             
                    {I2C_BOARD_INFO("lct2991", 0x4d),},                                                                                             
};
#endif
int lct2991_open (struct inode *inode, struct file *filp)
{
    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("===> %s\n", __func__));
    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("<=== %s\n", __func__));
	return 0;
}

ssize_t lct2991_read(struct file *file, char __user *buff, size_t count, loff_t *offp)
{
    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("===> %s\n", __func__));
	return 0;
}

ssize_t lct2991_write(struct file *file, const char __user *buff, size_t count, loff_t *offp)
{
    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("===> %s\n", __func__));
	return 0;
}

#if 0
static int lct2991_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	switch ( cmd ) {
		default:
			{
				break;
			}
	}
	return 0;
}
#endif
static int lct2991_release(struct inode *node, struct file *file)
{    
    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("===> %s\n", __func__));

	return 0;
}

static struct file_operations lct2991_i2c_fops = {
	.owner   = THIS_MODULE,
	.open    = lct2991_open,
	.release = lct2991_release,
	.read    = lct2991_read,
	.write   = lct2991_write,
//	.ioctl   = lct2991_ioctl,	
};

extern int i2c_master_send(const struct i2c_client *client, const char *buf, int count);
extern int i2c_master_recv(const struct i2c_client *client, char *buf, int count);

static int __devinit lct2991_probe(struct i2c_client *client,
		      const struct i2c_device_id *id)
{
    int ret = 0;
//    struct lct2991_data *stpdata;

    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("===> %s\n", __func__));

    new_client = client;
    devno = MKDEV(lct2991_major, LCT2991_MINOR);
    if (lct2991_major)
    {
        ret = register_chrdev_region(devno, number_of_devices, "ltc2991");
        PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("ret manual === %d \n", ret));
    }
    else
    {
	ret = alloc_chrdev_region(&devno, 0, number_of_devices, "ltc2991");
	lct2991_major = MAJOR(devno);
        PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("ret auto get devno=== %d \n", ret));
    }


    if(ret)
    {
        PRT_KI2C_DRIVER_ERROR(("failed to register lct2991 i2c device number\n"));
        goto err_register_chrdev_region;
    }
    cdev_init(&i2c_lct2991_dev, &lct2991_i2c_fops);

    i2c_lct2991_dev.owner = THIS_MODULE;


    ret = cdev_add(&i2c_lct2991_dev, devno, number_of_devices);
    if(ret)
    {
        PRT_KI2C_DRIVER_ERROR(("failed to add  lct2991 i2c  device\n"));
        goto err_cdev_add;
    }

    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("<=== %s\n", __func__));

    return 0;
err_cdev_add:
    unregister_chrdev_region(devno, number_of_devices);
err_register_chrdev_region:
//    kfree(pstdata);
    PRT_KI2C_DRIVER_ERROR(( " lct2991 err_register_chrdev_region !\n"));
    return ret;    
}

 /**
 * @brief Write a byte of data to register specified by "command"
 *
 * @param address 7位I2C 器件的地址
 * @param command 	Command byte 
 * @param value   Byte to be written
 *
 * @return 成功返回0 失败返回-1
 */
int8_t i2c_write_byte_data(uint8_t address, uint8_t command, uint8_t value)
{
    int ret=0;
    unsigned char buffer[2];
    buffer[0] = command;
    buffer[1] = value;

    struct i2c_msg msgs[] = {
        {
            .addr  = address,
            .flags = 0,
            .len   = 2,
            .buf   = buffer,
        },
    };

//    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("===> %s\n", __func__);
//    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("addr ===> 0x%x command ===> 0x%x value ===> 0x%x\n", address, command, value);

    new_client->addr = address;/* */

    ret = i2c_transfer(new_client->adapter, msgs, 1);    
//    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("ret ===> 0x%x\n", ret);
//
//    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("<=== %s\n", __func__);
    
    if(ret < 0 )
    {
        PRT_KI2C_DRIVER_ERROR((" %s:%s->%d i2c_transfer return failed\n", __func__, __FILE__, __LINE__));
        return -1;
    }
    else
    {
        return 0;
    }
#if 0
    if (2!= i2c_master_send(new_client,buffer,2) )
    {
        PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("lct2991 i2c write byte fail!\n");
        return -1;
    }
#endif
    return 0;
}
EXPORT_SYMBOL(i2c_write_byte_data);

/**
 * @brief 写一个字16位，ADC中寄存器的读取需要
 *
 * @param address  7位I2C 器件的地址
 * @param command  Command byte 
 * @param value    Word to be written
 *
 * @return  成功返回0 失败返回-1
 */
int8_t i2c_write_word_data(uint8_t address, uint8_t command, uint16_t value)
{
    uint8_t buf[3];
    int ret=0;

    struct i2c_msg msgs[] = {
        {
            .addr  = address,
            .flags = 0,
            .len   = 3,
            .buf   = buf,
        },
    };

    buf[0] = command;
    buf[1] = (uint8_t)value>>8;/*housir: 大端模式 */
    buf[2] = value & 0x00ff;/*housir: 大端 */

//    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("===> %s\n", __func__);
//    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("command ===> 0x%x\n", command);


    new_client->addr = address;/* */

    ret = i2c_transfer(new_client->adapter, msgs, 1);    
    if(ret < 0 )
    {
        PRT_KI2C_DRIVER_ERROR(("%s:%s->%d i2c_transfer return failed\n", __func__, __FILE__, __LINE__));
        return -1;
    }
    else
    {
        return 0;
    }
#if 0
    union
    {
        uint8_t buf[2];
        uint16_t word;
    } data;
    data.word = value;
//    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("");
    new_client->addr = address;/* */
    if (2!= i2c_master_send(new_client, &data, 2) )
    {
        PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("lct2991 i2c write word fail!\n");
        return -1;
    }
#endif
    return ret;
}
EXPORT_SYMBOL(i2c_write_word_data);

/**
 * @brief Read a byte, store in "value".
 *
 * @param address 7-bit I2C address 
 * @param command byte
 * @param value Byte to be read
 *
 * @return 0 on success, -1 on failure 
 */
int8_t i2c_read_byte_data(uint8_t address, uint8_t command, uint8_t *value)
{
    int ret = 0;
    struct i2c_msg msgs[] = {
        {
            .addr  = address,
            .flags = 0,
            .len   = 1,
            .buf   = &command,
        },
        {
            .addr  = address,
            .flags = I2C_M_RD,
            .len   = 1,
            .buf   = value,
        },
    };

//    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("===> %s\n", __func__);
//    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("command ===> 0x%x\n", command);


    new_client->addr = address;/* */

    ret = i2c_transfer(new_client->adapter, msgs, 2);
//    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("address ===> 0x%x\n value ===> 0x%x\n", address, *value);
//
//    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("<=== %s\n", __func__);

    if(ret < 0 )
    {
        PRT_KI2C_DRIVER_ERROR(("%s:%s->%d i2c_transfer return failed\n", __func__, __FILE__, __LINE__));
        return -1;
    }
    else
    {
        return 0;
    }
#if 0
    /*write reg addr  */
    if( 1!= i2c_master_send(new_client, command, 1) )
    {
        PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,( KERN_ERR " lct2991_i2c_read byte fail!:i2c_master_send \n" );
        return -1;
    }
    /*housir: wait 一个周期长多少？ */
    msleep(10);

    /*read */
    if( 1!= i2c_master_recv(new_client, value, 1) )
    {
        PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,( KERN_ERR " lct2991_i2c_read byte fail!:i2c_master_recv \n" );
        return -1;
    }
#endif

    return 0;
}
EXPORT_SYMBOL(i2c_read_byte_data);

/**
 * @brief Read a 16-bit word of data from register specified by "command". 
 *
 * @param address 	7-bit I2C address 
 * @param command   Command byte 
 * @param value     Word to be read 
 *
 * @return 0 on success, -1 on failure 
 */
int8_t i2c_read_word_data(uint8_t address, uint8_t command, uint16_t *value)
{
#if 0
    union
    {
        uint8_t buf[2];
        uint16_t word;
    } data;
    data.word = 0;
#endif
    int ret=0;
    struct i2c_msg msgs[] = {
        {
            .addr  = address,
            .flags = 0,
            .len   = 1,
            .buf   = &command,
        },
        {
            .addr  = address,
            .flags = I2C_M_RD,
            .len   = 2,
            .buf   = value,
        },
    };

//    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("===> %s\n", __func__);
//    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("address ===> 0x%x\n command ===> 0x%x\n", address, command);


    new_client->addr = address;/* */

    ret = i2c_transfer(new_client->adapter, msgs, 2);
//    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("address === 0x%x\n value === 0x%x\n", address, *value);
//    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("msg[1].buf === 0x%x\n ", *(msgs[1].buf));

    if(ret < 0 )
    {
        PRT_KI2C_DRIVER_ERROR(("%s:%s->%d i2c_transfer return failed\n", __func__, __FILE__, __LINE__));
        return -1;
    }
    else
    {
        return 0;
    }
#if 0
    new_client->addr = address;/* */
    /*write reg addr  */
    if( 1!= i2c_master_send(new_client, command, 1) )
    {
        PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,( KERN_ERR " lct2991_i2c_read word fail! :master_send\n" );
        return -1;
    }
    /*housir: wait 一个周期长多少？ */
//    msleep(10);

    /*read */
    if( 2!= i2c_master_recv(new_client, value, 2) )
    {
        PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,( KERN_ERR " lct2991_i2c_read word fail! :master_recv\n" );
        return -1;
    }
#endif
    return ret;
}
EXPORT_SYMBOL(i2c_read_word_data);

static int lct2991_remove(struct i2c_client *client)
{
//    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("===> %s\n", __func__);

    cdev_del(&i2c_lct2991_dev);
    unregister_chrdev_region(devno, number_of_devices);
//	PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("lct2991_i2c device uninstalled\n");
    
    return 0;
}

static int __init lct2991_init(void)
{      
    int ret=0;

//    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("===> %s\n", __func__);

//    i2c_register_board_info(0, i2c_devs0, ARRAY_SIZE(i2c_devs0));/*housir:  没有这个设备，则需要提前注册上*/   
    /*housir: 将name和id_table 上报: i2c_add_driver会将驱动注册到总线上，（2）探测到i2c设备就会调用at24c02_probe， （3）探测主要是用i2c_match_id函数比较client的名字和id_table中名字，如果相等，则探测到i2c设备 */
    ret =  i2c_add_driver(&lct2991_i2c_driver);

//    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("return by i2c_add_driver === %d\n", ret);
//    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("<=== %s\n", __func__);
//    return i2c_add_driver(&lct2991_i2c_driver);
    return ret;
}
 
static void __exit lct2991_exit(void)
{
//    PRT_KI2C_DRIVER_DEBUG(K_DEBUG_INFO,("===> %s\n", __func__));
    i2c_del_driver(&lct2991_i2c_driver);
}


module_init(lct2991_init);
module_exit(lct2991_exit);

//module_i2c_driver(lct2991_i2c_driver);

MODULE_AUTHOR("housir <houwentaoff@gmail.com>");
MODULE_DESCRIPTION("LCT2991 driver");
MODULE_LICENSE("GPL");
