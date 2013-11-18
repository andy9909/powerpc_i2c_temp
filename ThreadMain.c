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

#define FPGA_READ_ADDR          0xf0000000             /*FPGA需要读的地址  */
#define FPGA_READ_SIZE         400            /*  */
#define NEGATIVE       1            /*  负值*/
#define POSITIVE       0            /* 正值 */

#define SENSOR_V_MAX_NUM       6            /*传感器中测的电压的个数  */
#define SENSOR_T_MAX_NUM       4            /*传感器中测的温度的个数  */

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Kozo");

 const int LTC2991_SINGLE_ENDED_lsb = 305;/*housir: 0.000305 * 13bit adc_code */
//! Typical differential LSB weight in volts
 const int LTC2991_DIFFERENTIAL_lsb = 19;/*housir: 0.00019 * 13bit adc_code*/
//! Typical VCC LSB weight in volts
// const int LTC2991_VCC_lsb = 3.05176E-04;
//! Typical temperature LSB weight in degrees Celsius (and Kelvin).
//! Used for internal temperature as well as remote diode temperature measurements.
 const int LTC2991_TEMPERATURE_lsb = 625;
//! Typical remote diode LSB weight in volts.
//! Used to readback diode voltage when in temperature measurement mode.
// const int LTC2991_DIODE_VOLTAGE_lsb = 3.815E-05;
 const uint16_t LTC2991_TIMEOUT =1000;

typedef struct lct2991_v_temp_data_tag{
    int16_t adc_code;/*housir: 温度采集到的adc_code或者电压采集到的code */
    uint8_t sign ;   /*housir: 符号位 */ 
    int hvtemp ;     /*housir: 温度/电压的整数位 */
    int lvtemp ;     /*housir: 温度/电压的小数位 */    
 }st_sensorinfo,*pst_sensorinfo;
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

    int8_t data_valid;
    int temperature = 0;
    int voltage = 0;
    int8_t ack = 0;
        /*housir: 使用到的温度传感器的寄存器表 */
//    const uint8_t aVmsbreg[]={LTC2991_V1_MSB_REG, LTC2991_V3_MSB_REG,LTC2991_V5_MSB_REG,LTC2991_V7_MSB_REG};
    const uint8_t aVnum_Creg[]={LTC2991_CONTROL_V1234_REG, LTC2991_CONTROL_V1234_REG,LTC2991_CONTROL_V5678_REG,LTC2991_CONTROL_V5678_REG};
    const uint8_t aVnum_Tenablereg[]={LTC2991_V1_V2_TEMP_ENABLE, LTC2991_V3_V4_TEMP_ENABLE,LTC2991_V5_V6_TEMP_ENABLE,LTC2991_V7_V8_TEMP_ENABLE};
    /*housir: 采集电压需要使用到的表 */
    uint8_t av_clearbit[]={
        LTC2991_V1_V2_DIFFERENTIAL_ENABLE | LTC2991_V1_V2_TEMP_ENABLE, LTC2991_V1_V2_DIFFERENTIAL_ENABLE | LTC2991_V1_V2_TEMP_ENABLE,
        LTC2991_V3_V4_DIFFERENTIAL_ENABLE | LTC2991_V3_V4_TEMP_ENABLE, LTC2991_V3_V4_DIFFERENTIAL_ENABLE | LTC2991_V3_V4_TEMP_ENABLE,
        LTC2991_V5_V6_DIFFERENTIAL_ENABLE | LTC2991_V5_V6_TEMP_ENABLE, LTC2991_V5_V6_DIFFERENTIAL_ENABLE | LTC2991_V5_V6_TEMP_ENABLE,
    };/*housir: 设置为采集电压模式，需要清除的位 */
    uint8_t vn_msb_reg_base = LTC2991_V1_MSB_REG;/*housir: MSB的基地址 */

    uint8_t value = 0;
    uint8_t write_value = 0x00;
    
    uint8_t *pos = NULL;
    uint8_t *temp_pos =NULL;

    st_sensorinfo stsensor =
    {
        .adc_code = 0,
        .sign     = POSITIVE,
        .hvtemp   = 0,
        .lvtemp   = 0,
    };

    printk("---> %s\n", __func__);

    pos = (uint8_t *)ioremap(FPGA_READ_ADDR, FPGA_READ_SIZE);

    temp_pos = pos;
	/*在此完成一些初始化动作*/

	while(1)
	{
		printk("my thread:current->mm = %p,index = 0x%x\n",current->mm,index++);
		printk("my thread:current->active_mm = %p\n",current->active_mm);
		set_current_state(TASK_INTERRUPTIBLE);
		/*指定周期内将线程唤醒*/
		schedule_timeout(10*HZ);
#if 0 /*测试读和写FPGA的值  */
//       for(i=0;i++;i<400)
       {
            printk("fpga 0xf0000000 + i*4 : 0x%x\n", *(int *)pos);
            pos += 4;
       }
#endif
#if 0  /*housir:  测试代码，对2个i2c从设备LCT2991 reg 0x07 的周期读写*/
		/*在此可以完成一些监控动作*/
		/*完成电压监控从LTC2991读取6路电压写入fpga寄存器*/

        i2c_write_byte_data(0x4d, LTC2991_CONTROL_V5678_REG, write_value);
        msleep(100);
        i2c_read_byte_data(0x4d, LTC2991_CONTROL_V5678_REG, &value);

        printk("slave addr:0x4d reg:0x07: write_value %d \n read value == %d\n", write_value, value);

        i2c_write_byte_data(0x4b, LTC2991_CONTROL_V5678_REG, write_value);
//        msleep(100);
        i2c_read_byte_data(0x4b, LTC2991_CONTROL_V5678_REG, &value);

        printk("slave addr:0x4b reg:0x07: write_value %d \n read value == %d\n", write_value, value);

        value = 0;
        write_value++;
#endif
#if 1  /*housir:  读所有温度和电压值;*/
        /*Read temperature from diode connected to V7-V8.*/
        /* Enable temperature mode.*/
        ack = LTC2991_register_set_clear_bits(LTC2991_I2C_TEMP_ADDRESS, LTC2991_CONTROL_V5678_REG, LTC2991_V7_V8_TEMP_ENABLE, 0x00);

        /* Flush one ADC reading in case it is stale.  Then, take a new fresh reading.*/
        ack = LTC2991_adc_read_new_data(LTC2991_I2C_TEMP_ADDRESS, LTC2991_V7_MSB_REG, &stsensor.adc_code, &data_valid, LTC2991_TIMEOUT);

        /* Converts code to temperature from adc code and temperature lsb */
        temperature = LTC2991_temperature(stsensor.adc_code, LTC2991_TEMPERATURE_lsb, 1);

        printk("temp_org === %d\n", temperature);

        stsensor.sign = temperature & 0x80000000;
        temperature &= ~(0x80000000);/*housir: 符号位置0 */

        stsensor.hvtemp = temperature/1000;
        stsensor.lvtemp = temperature%1000; //- (int)temperature*100;

        printk("read v7 v8 tem is %d.%d\n", stsensor.hvtemp , stsensor.lvtemp );

        temperature = 0;
        stsensor.adc_code = 0;
#endif       
#if 0
        for (index = 0;index < SENSOR_T_MAX_NUM;index++)
        {
            /*Read temperature from diode connected to V1-V2 V3-V4 V5-V6 V7-V8.*/

            ack = LTC2991_register_set_clear_bits(LTC2991_I2C_TEMP_ADDRESS, aVnum_Creg[index], aVnum_Tenablereg[index], 0x00);

            /* Flush one ADC reading in case it is stale.  Then, take a new fresh reading.*/
            ack = LTC2991_adc_read_new_data(LTC2991_I2C_TEMP_ADDRESS, vn_msb_reg_base, &stsensor.adc_code, &data_valid, LTC2991_TIMEOUT);

            /* Converts code to temperature from adc code and temperature lsb */
            temperature = LTC2991_temperature(stsensor.adc_code, LTC2991_TEMPERATURE_lsb, 1);

            printk("temp_org === %d\n", temperature);

            stsensor.sign = temperature & 0x80000000;
            temperature &= ~(0x80000000);/*housir: 符号位置0 */

            stsensor.hvtemp = temperature/1000;
            stsensor.lvtemp = temperature%1000; //- (int)temperature*100;

            printk("read v%d v%d tem is %d.%d\n", 2*index-1, 2*index, stsensor.hvtemp , stsensor.lvtemp );

            temperature = 0;
            stsensor.adc_code = 0;
            vn_msb_reg_base += 0x04;/*housir: V1 3 5 7 之间的MSB基地址偏移量 */

        }
#endif
#if 0
/*Read single-ended voltage from V1.*/

    // Enable Single-Ended Mode
    ack |= LTC2991_register_set_clear_bits(LTC2991_I2C_ADDRESS, LTC2991_CONTROL_V1234_REG, 0x00, LTC2991_V1_V2_DIFFERENTIAL_ENABLE | LTC2991_V1_V2_TEMP_ENABLE);

    // Flush one ADC reading in case it is stale.  Then, take a new fresh reading.
    ack |= LTC2991_adc_read_new_data(LTC2991_I2C_V_ADDRESS, LTC2991_V1_MSB_REG, &code, &data_valid, LTC2991_TIMEOUT);

    voltage = LTC2991_code_to_single_ended_voltage(code, LTC2991_SINGLE_ENDED_lsb); // Converts code to voltage from single-ended lsb
#endif
#if 0
/*Read single-ended voltage from V1.*/

    vn_msb_reg_base = LTC2991_V1_MSB_REG;

    for (index = 0;index < SENSOR_V_MAX_NUM;index++)
    {
        /*Read single-ended voltage from V($index).*/

        ack |= LTC2991_register_set_clear_bits(LTC2991_I2C_V_ADDRESS, aVnum_Creg[index], 0x00, av_clearbit[index]);

        ack |= LTC2991_adc_read_new_data(LTC2991_I2C_V_ADDRESS, vn_msb_reg_base, &stsensor.adc_code, &data_valid, LTC2991_TIMEOUT);

        voltage = LTC2991_code_to_single_ended_voltage(stsensor.adc_code, LTC2991_SINGLE_ENDED_lsb); // Converts code to voltage from single-ended lsb
        vn_msb_reg_base += 0x02;/*housir: v(1-6)相邻地址的增量:0x02 */

    }

#endif		
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
