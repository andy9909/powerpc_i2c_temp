/*
 * =====================================================================================
 *       Copyright (c), 2013-2020, Prophet C&S.
 *       Filename:  ThreadMain.c
 *
 *    Description:  循环读取多个温控点的温度值并写入FPGA中
 *         Others:  
 *
 *        Version:  1.0
 *        Date:  Monday, November 11, 2013 02:11:19 HKT
 *       Revision:  none
 *       Compiler:  PowerPc-gcc
 *
 *         Author:  housir houwentaoff@gmail.com
 *   Organization:  Prophet
 *
 * =====================================================================================
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>

#include "i2c_ltc2991.h"
#include "LTC2991.h"


MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Kozo");

 const int LTC2991_SINGLE_ENDED_lsb = 3.05176E-04;
//! Typical differential LSB weight in volts
 const int LTC2991_DIFFERENTIAL_lsb = 1.90735E-05;
//! Typical VCC LSB weight in volts
 const int LTC2991_VCC_lsb = 3.05176E-04;
//! Typical temperature LSB weight in degrees Celsius (and Kelvin).
//! Used for internal temperature as well as remote diode temperature measurements.
 const int LTC2991_TEMPERATURE_lsb = 625;
//! Typical remote diode LSB weight in volts.
//! Used to readback diode voltage when in temperature measurement mode.
 const int LTC2991_DIODE_VOLTAGE_lsb = 3.815E-05;
 const uint16_t LTC2991_TIMEOUT =1000;

/*************************************************************
函数名:CreatThreadMain
功能:创建设备功能主线程
输入参数:
输出参数:
作者: 聂飞
时间:
注意事项:
*************************************************************/
static int CreatThreadMain(void *v_ptr) 
{
	int index = 0;
	v_ptr = v_ptr;
    int16_t adc_code;
    int8_t data_valid;
    int temperature = 0;
    int8_t ack = 0;
    int htemp = 0;/*housir: 温度的整数位 */
    int ltemp = 0;/*housir: 温度的小数位 */
    uint8_t value = 0;
    uint8_t write_value = 0x00;

    printk("---> %s\n", __func__);
      printk("sizeof (int) %d\n", sizeof(int));

	/*在此完成一些初始化动作*/

	while(1)
	{
		printk("my thread:current->mm = %p,index = 0x%x\n",current->mm,index++);
		printk("my thread:current->active_mm = %p\n",current->active_mm);
		set_current_state(TASK_INTERRUPTIBLE);
		/*指定周期内将线程唤醒*/
		schedule_timeout(10*HZ);
		/*在此可以完成一些监控动作*/
		/*完成电压监控从LTC2991读取6路电压写入fpga寄存器*/

        i2c_write_byte_data(0x4d, LTC2991_CONTROL_V5678_REG, write_value);
        msleep(100);
        i2c_read_byte_data(0x4d, LTC2991_CONTROL_V5678_REG, &value);

        printk("write_value %d \n read value == %d\n", write_value, value);
        value = 0;
        write_value++;
#if 0
        /*Read temperature from diode connected to V7-V8.*/
        /* Enable temperature mode.*/
        ack |= LTC2991_register_set_clear_bits(LTC2991_I2C_ADDRESS, LTC2991_CONTROL_V5678_REG, LTC2991_V7_V8_TEMP_ENABLE, 0x00);
        

        /* Flush one ADC reading in case it is stale.  Then, take a new fresh reading.*/
        ack |= LTC2991_adc_read_new_data(LTC2991_I2C_ADDRESS, LTC2991_V7_MSB_REG, &adc_code, &data_valid, LTC2991_TIMEOUT);

        /* Converts code to temperature from adc code and temperature lsb */
        temperature = LTC2991_temperature(adc_code, LTC2991_TEMPERATURE_lsb, 1);

        printk("temp_org === %d\n", temperature);

        htemp = temperature/1000;
        ltemp = temperature%1000; //- (int)temperature*100;

        printk("read v7 v8 tem is %d.%d\n", htemp, ltemp);

        temperature = 0;
#endif
#if 0
		LTC2991_adc_read(0X4D, msb_register_address, &adc_code, &data_valid);
        LTC2991_temperature(adc_code, LTC2991_TEMPERATURE_LSB);
#endif
		/*温度监控*/
		
	}
	return 0;
}

static __init int ThreadMain_init(void)
{

	printk(KERN_INFO"demo init\n");
	printk("demo init:current->mm = %p\n",current->mm);
	printk("demo init:current->active_mm = %p\n",current->active_mm);
	/*iomap*/
	kernel_thread(CreatThreadMain, NULL, CLONE_KERNEL|SIGCHLD);
	//kthread_run(noop2,NULL,"mythread");
	return 0;
}

static __exit void ThreadMain_exit(void)
{
	printk(KERN_INFO"demo exit\n");
}

module_init(ThreadMain_init);
module_exit(ThreadMain_exit);
