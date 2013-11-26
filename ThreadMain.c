/*
 * =====================================================================================
 *       Copyright (c), 2013-2020, Prophet C&S.
 *       Filename:  ThreadMain.c
 *
 *    Description:  ѭ����ȡ����¿ص���¶�ֵ��д��FPGA��
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
#include <linux/fs.h>
#include <asm/uaccess.h>

#include "i2c_ltc2991.h"
#include "LTC2991.h"

#define FPGA_TEMP_BASE_ADDR          0xf0000000             /*FPGA��Ҫ���ĵ�ַ  */
#define FPGA_TEMP_WRITE_SIZE         4*4            /*  */
#define FPGA_V_WRITE_SIZE         4*4            /*  */
#define NEGATIVE       1            /*  ��ֵ*/
#define POSITIVE       0            /* ��ֵ */

#define SENSOR_V_MAX_NUM       6            /*�������в�ĵ�ѹ�ĸ���  */
#define SENSOR_T_MAX_NUM       4            /*�������в���¶ȵĸ���  */
#define LTC2991_V_DELAY_MS      2                    /* Single-Ended Voltage Measurement l 0.9 1.5 1.8  */
#define LTC2991_TEMP_DELAY_MS    10        /* �¶�����û��˵����ʱ�೤,������Ϊ10 */
#define TOTAL_SENSOES            10/*һ��10���������ɼ���*/
#define SENSOR_TEMP_TOTAL           4            /*4���¶Ȳɼ���  */

#define      MAJOR_NUM    244       /*  */
#if 0
#define TEMP_1_ADDR            /*  */
#define TEMP_2_ADDR            /*  */
#define TEMP_3_ADDR            /*  */
#define TEMP_4_ADDR            /*  */

#define V_1_ADDR            /*  */
#define V_2_ADDR            /*  */
#define V_3_ADDR            /*  */
#define V_4_ADDR            /*  */
#define V_5_ADDR            /*  */
#define V_6_ADDR            /*  */
#endif


/*housir: ���� */
 const int LTC2991_SINGLE_ENDED_lsb = 305;/*housir: 0.000305 * 13bit adc_code */
 const int LTC2991_DIFFERENTIAL_lsb = 19;/*housir: 0.00019 * 13bit adc_code*/
 const int LTC2991_TEMPERATURE_lsb = 625;
 const uint16_t LTC2991_TIMEOUT =1000;

typedef struct lct2991_v_temp_data_tag{
//    int16_t adc_code;/*housir: �¶Ȳɼ�����adc_code���ߵ�ѹ�ɼ�����code */
    uint8_t sign ;   /*housir: ����λ */ 
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
static int sensor_major = 244;
static struct cdev sensor_read_cdev;


/*housir: �������� */
static int sensor_open(struct inode *inode, struct file *filp);
static ssize_t sensor_read(struct file *filp, char __user *buffer, size_t count, loff_t *ppos);
   

static struct file_operations sensor_read_ops = {
	owner:	THIS_MODULE,
	open:	sensor_open,
	read:	sensor_read,	
};

static int sensor_open(struct inode *inode, struct file *filp)
{
//    printk("===> %s\n", __func__);
    return 0;
}
/*************************************************************
������:CreatThreadMain
����:�����豸�������߳�
�������:
�������:
����: ����
ʱ��:
ע������:
*************************************************************/
static int CreatThreadMain(void *v_ptr) 
{
	v_ptr = v_ptr;
    int8_t ack = 0;

#if 0 
        /*housir: ʹ�õ����¶ȴ������ļĴ����� */
//    const uint8_t aVmsbreg[]={LTC2991_V1_MSB_REG, LTC2991_V3_MSB_REG,LTC2991_V5_MSB_REG,LTC2991_V7_MSB_REG};
    const uint8_t aVnum_Tenablereg[]={LTC2991_V1_V2_TEMP_ENABLE, LTC2991_V3_V4_TEMP_ENABLE,LTC2991_V5_V6_TEMP_ENABLE,LTC2991_V7_V8_TEMP_ENABLE};
    /*housir: �ɼ���ѹ��Ҫʹ�õ��ı� */
    uint8_t av_clearbit[]={
        LTC2991_V1_V2_DIFFERENTIAL_ENABLE | LTC2991_V1_V2_TEMP_ENABLE, LTC2991_V1_V2_DIFFERENTIAL_ENABLE | LTC2991_V1_V2_TEMP_ENABLE,
        LTC2991_V3_V4_DIFFERENTIAL_ENABLE | LTC2991_V3_V4_TEMP_ENABLE, LTC2991_V3_V4_DIFFERENTIAL_ENABLE | LTC2991_V3_V4_TEMP_ENABLE,
        LTC2991_V5_V6_DIFFERENTIAL_ENABLE | LTC2991_V5_V6_TEMP_ENABLE, LTC2991_V5_V6_DIFFERENTIAL_ENABLE | LTC2991_V5_V6_TEMP_ENABLE,
    };/*housir: ����Ϊ�ɼ���ѹģʽ����Ҫ�����λ */
#endif
    
//    volatile unsigned int *pos= NULL;
//    uint8_t *temp_pos =NULL;

//    printk("---> %s\n", __func__);

#if 0 /*housir: FPGAд���� */
    printk("===> temp write begin\n");
    pos = (unsigned int *)ioremap(FPGA_TEMP_BASE_ADDR + 0x7c, FPGA_TEMP_WRITE_SIZE);

    for (i=0;i<FPGA_TEMP_WRITE_SIZE;i++)
    {
        *pos = i;
        printk("0x%x",*pos);
    }
    printk("\n<=== temp write over\n");

    printk("===> v fpga addr write begin\n");
    pos = (unsigned int *)ioremap(FPGA_TEMP_BASE_ADDR + 0xa0, FPGA_V_WRITE_SIZE);

    for (i=0;i<FPGA_V_WRITE_SIZE;i++)
    {
        *pos = i;
        printk("0x%x",*pos);
    }
    printk("\n<=== v fpga addr write over\n");
 
    temp_pos = pos;
#endif
#if 0
	/*�ڴ����һЩ��ʼ������*/
/*housir: һ�����ö�ζ�ȡ,�ǲ��ǲɼ�����ֵ̫�磬���õ�̫��Ƶ�������¶����� ����Ϊ0 */
    i2c_read_byte_data(0x4d, LTC2991_CONTROL_V5678_REG, &value);
    printk("before set LTC2991_CONTROL_V5678_REG %d\n", value);
    value = 0;
    LTC2991_register_set_clear_bits(LTC2991_I2C_TEMP_ADDRESS, LTC2991_CONTROL_V5678_REG, 
            LTC2991_V7_V8_TEMP_ENABLE | LTC2991_V5_V6_TEMP_ENABLE, 0x00);
    i2c_read_byte_data(0x4d, LTC2991_CONTROL_V5678_REG, &value);
    printk("after set LTC2991_CONTROL_V5678_REG %d\n", value);

    value = 0;
    i2c_read_byte_data(0x4d, LTC2991_CONTROL_V1234_REG, &value);
    printk("before set LTC2991_CONTROL_V1234_REG %d\n", value);
    value = 0;
    LTC2991_register_set_clear_bits(LTC2991_I2C_TEMP_ADDRESS, LTC2991_CONTROL_V1234_REG, 
            LTC2991_V1_V2_TEMP_ENABLE | LTC2991_V3_V4_TEMP_ENABLE, 0x00);
    i2c_read_byte_data(0x4d, LTC2991_CONTROL_V1234_REG, &value);
    printk("after set LTC2991_CONTROL_V1234_REG %d\n", value);

#endif
    ack |= LTC2991_register_set_clear_bits(LTC2991_I2C_V_ADDRESS, LTC2991_CONTROL_V1234_REG, 0x00, LTC2991_V1_V2_DIFFERENTIAL_ENABLE | LTC2991_V1_V2_TEMP_ENABLE | LTC2991_V3_V4_DIFFERENTIAL_ENABLE | LTC2991_V3_V4_TEMP_ENABLE );

    ack |= LTC2991_register_set_clear_bits(LTC2991_I2C_V_ADDRESS, LTC2991_CONTROL_V5678_REG, 0x00, LTC2991_V5_V6_DIFFERENTIAL_ENABLE | LTC2991_V5_V6_TEMP_ENABLE );

    LTC2991_register_set_clear_bits(LTC2991_I2C_TEMP_ADDRESS, LTC2991_CONTROL_V1234_REG, 
            LTC2991_V1_V2_TEMP_ENABLE | LTC2991_V3_V4_TEMP_ENABLE, 0x00);
    LTC2991_register_set_clear_bits(LTC2991_I2C_TEMP_ADDRESS, LTC2991_CONTROL_V5678_REG, 
            LTC2991_V5_V6_TEMP_ENABLE | LTC2991_V7_V8_TEMP_ENABLE, 0x00);
    /*housir: ��ѹ */
   if  (0 != i2c_write_byte_data(LTC2991_I2C_V_ADDRESS, LTC2991_CHANNEL_ENABLE_REG , 0xf0))
    {
        printk("===> %s write  reg_addr 0x01 error!!!\n", __func__);
    }
/*housir: �¶� */
   if  (0 != i2c_write_byte_data(LTC2991_I2C_TEMP_ADDRESS, LTC2991_CHANNEL_ENABLE_REG , 0xf0))
    {
        printk("===> %s write  reg_addr 0x01 error!!!\n", __func__);
    }
#if 0
	while(1)
	{
//		printk("my thread:current->mm = %p,index = 0x%x\n",current->mm, thread_index++);
//		printk("my thread:current->active_mm = %p\n",current->active_mm);
		set_current_state(TASK_INTERRUPTIBLE);
		/*ָ�������ڽ��̻߳���*/
		schedule_timeout(10*HZ);

		/*�ڴ˿������һЩ��ض���*/
		/*��ɵ�ѹ��ش�LTC2991��ȡ6·��ѹд��fpga�Ĵ���*/

	}
#endif
	return 0;
}

static ssize_t sensor_read(struct file *filp, char __user *buffer, size_t count, loff_t *ppos)
{
    int index = 0;
    int thread_index = 0;
    int16_t adc_code;/*housir: �¶Ȳɼ�����adc_code���ߵ�ѹ�ɼ�����code */
    int8_t data_valid;
    int temperature = 0;
    int voltage = 0;
    int8_t ack = 0;
    uint8_t vn_msb_reg_base = LTC2991_V1_MSB_REG;/*housir: MSB�Ļ���ַ */

    uint8_t value = 0;
    uint8_t write_value = 0x00;
    uint8_t i =0;
 //   printk("===> %s\n", __func__);    
#if  0
    for( index=0;index<SENSOR_TEMP_TOTAL;index++)
    {
        printk("read v%d v%d tem is %d.%d\n", 2*index+1, 2*index+2, stasensor_value[index].hvtemp , stasensor_value[index].lvtemp );
    }
    for( index=0;index<SENSOR_V_MAX_NUM;index++)
    {
        printk("read v%d V is %d.%d\n", index+1, stasensor_value[ index + SENSOR_TEMP_TOTAL ].hvtemp ,  stasensor_value[ index + SENSOR_TEMP_TOTAL ].lvtemp );
    }
#endif
#if 1
        vn_msb_reg_base = LTC2991_V1_MSB_REG;/*housir: MSB�Ļ���ַ */

        for (index = 0;index < SENSOR_T_MAX_NUM;index++)
        {
            /*Read temperature from diode connected to V1-V2 V3-V4 V5-V6 V7-V8.*/
//            ack = LTC2991_register_set_clear_bits(LTC2991_I2C_TEMP_ADDRESS, aVnum_Creg[index], aVnum_Tenablereg[index], 0x00);
            /* Flush one ADC reading in case it is stale.  Then, take a new fresh reading.*/
            ack = LTC2991_adc_read_new_data(LTC2991_I2C_TEMP_ADDRESS, vn_msb_reg_base, &adc_code, &data_valid, LTC2991_TEMP_DELAY_MS);
            /* Converts code to temperature from adc code and temperature lsb */
            temperature = LTC2991_temperature(adc_code, LTC2991_TEMPERATURE_lsb, 1);

            stasensor_value[index].sign = temperature & 0x80000000;
            temperature = temperature>0 ? temperature:0-temperature;/*housir: ����λ��0 */

            stasensor_value[index].hvtemp = temperature/10000;
            
	        stasensor_value[index].lvtemp = (temperature%10000)/100; //- (int)temperature*100;

 //           printk("kthread : read v%d v%d tem is %d.%.2d\n", 2*index+1, 2*index+2, stasensor_value[index].hvtemp , stasensor_value[index].lvtemp );


            temperature = 0;
            adc_code = 0;
            vn_msb_reg_base += 0x04;/*housir: V1 3 5 7 ֮���MSB����ַƫ���� */
        }
#endif

#if 1
/*Read single-ended voltage from V1.*/
     vn_msb_reg_base = LTC2991_V1_MSB_REG;

     //LTC2991_register_set_clear_bits(LTC2991_I2C_V_ADDRESS, , 0x00, av_clearbit[index]);
    for (index = 0;index < SENSOR_V_MAX_NUM;index++)
    {
        /*Read single-ended voltage from V($index).*/
//        ack |= LTC2991_register_set_clear_bits(LTC2991_I2C_V_ADDRESS, aVnum_Creg[index], 0x00, av_clearbit[index]);

        ack |= LTC2991_adc_read_new_data(LTC2991_I2C_V_ADDRESS, vn_msb_reg_base, &adc_code, &data_valid, LTC2991_V_DELAY_MS);
        voltage = LTC2991_code_to_single_ended_voltage(adc_code, LTC2991_SINGLE_ENDED_lsb); // Converts code to voltage from single-ended lsb
        stasensor_value[ index + SENSOR_TEMP_TOTAL ].sign = voltage & 0x80000000;
        voltage = voltage>0 ? voltage:0-voltage;/*housir: ����λ��0 */

        stasensor_value[index + SENSOR_TEMP_TOTAL].hvtemp = voltage/1000000;

 //       printk("voltage v[%d] : [%d]\n", index+1, voltage);

        stasensor_value[index + SENSOR_TEMP_TOTAL].lvtemp = (voltage%1000000)/10000; //- (int)temperature*100;
        
        //printk("kthread : read v%d V is %d.%0.2d\n", index+1, stasensor_value[ index + SENSOR_TEMP_TOTAL ].hvtemp ,  stasensor_value[ index + SENSOR_TEMP_TOTAL ].lvtemp );
        adc_code = 0;
        voltage = 0;
        vn_msb_reg_base += 0x02;/*housir: v(1-6)���ڵ�ַ������:0x02 */
    }

#endif		
    /*housir:����һ�� ����һƬ�ڴ�copy���û��ռ� */
    if(copy_to_user(buffer, (char *)&stasensor_value[0], sizeof(stasensor_value)))
    {
		return -EFAULT;//�����ں����ݵ��û��ռ�
    }
    /*housir: ���������������洫���count ���ɲ�������ֵ copy��ǰָ����������ֵ */
#if 0
	if(copy_to_user(buffer, (char *)&stasensor_value[count], sizeof(stasensor_value)))
    {
		return -EFAULT;//�����ں����ݵ��û��ռ�
    }   
#endif

 //   printk("<=== %s\n", __func__);    
}
#if 0
/*
 *��ʼ������ӽṹ��struct cdev��ϵͳ֮��
 */
static void sensor_setup_cdev(struct cdev *dev, int minor,
		struct file_operations *fops)
{
	int err, devno = MKDEV(adc_major, minor);
    
	cdev_init(dev, fops);//��ʼ���ṹ��struct cdev
	dev->owner = THIS_MODULE;
	dev->ops = fops;//���ṹ�����ops��Ա����ֵ�������Ƕ��豸�����ľ����ʵ�ֺ���
	err = cdev_add (dev, devno, 1);//���ṹ��struct cdev��ӵ�ϵͳ֮��
	/* Fail gracefully if need be */
	if (err)
    {
		printk (KERN_NOTICE "Error %d adding adc %d", err, minor);
    }
}
#endif

static  int __init ThreadMain_init(void)
{

	int result;
    int err=0;

//	printk(KERN_INFO"demo init\n");
//	printk("demo init:current->mm = %p\n",current->mm);
//	printk("demo init:current->active_mm = %p\n",current->active_mm);

    register_chrdev(MAJOR_NUM, "read_sensor", &sensor_read_ops);/*housir: ��ʽ�ӿ� ���������cdev_add��*/
#if 0
	dev_t dev = MKDEV(sensor_major, 0);//�����豸�źʹ��豸�Ŷ��嵽һ��dev_t�������͵Ľṹ��֮��

	/* ��ʼ���ַ��豸 */
	if (sensor_major)
    {
		result = register_chrdev_region(dev, 1, "read_sensor");//��̬ע��һ���豸���豸����ǰָ���ã����õ�һ���豸����cat /proc/device���鿴��Ϣ
    }
    else 
    {
		result = alloc_chrdev_region(&dev, 0, 1, "read_sensor");//������豸�ű�ռ�ã�����ϵͳ�ṩһ�����豸�Ÿ��豸��������
		sensor_major = MAJOR(dev);//�õ����豸��
	}
	if (result < 0) {
		return result;
	}
	if (sensor_major == 0)                          
		sensor_major = result;//�����̬����ʧ�ܡ��Ѷ�̬������豸�Ÿ��豸��������

	cdev_init(&sensor_read_cdev, &sensor_read_ops);//��ʼ���ṹ��struct cdev
	sensor_read_cdev.owner = THIS_MODULE;
//	dev->ops = &sensor_read_ops;//���ṹ�����ops��Ա����ֵ�������Ƕ��豸�����ľ����ʵ�ֺ�����cdev_init���Ѿ�����ֵ��
	err = cdev_add (&sensor_read_cdev, 0, 1);//���ṹ��struct cdev��ӵ�ϵͳ֮��,�м��0����Ӧ����dev
	err = cdev_add (&sensor_read_cdev, dev, 1);//���ṹ��struct cdev��ӵ�ϵͳ֮��,�м��0����Ӧ����dev
	/* Fail gracefully if need be */
	if (err)
    {
		printk (KERN_NOTICE "Error %d \n", err);
    }
    
//	adc_setup_cdev(&AdcDevs, 0, &adc_remap_ops);//��ʼ������ӽṹ��struct cdev��ϵͳ֮��
	/*iomap*/
#endif
    CreatThreadMain(NULL);
//	kernel_thread(CreatThreadMain, NULL, CLONE_KERNEL|SIGCHLD);
	//kthread_run(noop2,NULL,"mythread");
	return 0;
}

static  void __exit ThreadMain_exit(void)
{
#if 0
    cdev_del(&sensor_read_cdev);
	unregister_chrdev_region(MKDEV(sensor_major, 0), 1);//ж���豸������ռ�е���Դ
#endif
   unregister_chrdev(MAJOR_NUM, "read_sensor");/*housir: ��ʽ�Ľӿڣ������е���cdev_del */
   printk(KERN_INFO"demo exit\n");
}

module_init(ThreadMain_init);
module_exit(ThreadMain_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("housir");
