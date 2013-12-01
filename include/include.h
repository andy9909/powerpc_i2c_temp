/*
 * =====================================================================================
 *       Copyright (c), 2013-2020, Prophet C&S.
 *       Filename:  include.h
 *
 *    Description:  温度数据结构的定义
 *         Others:
 *   
 *        Version:  1.0
 *        Date:  Tuesday, November 26, 2013 11:16:41 HKT
 *       Revision:  none
 *       Compiler:  powerpc-gcc
 *
 *         Author:  housir , houwentaoff@gmail.com
 *   Organization:  Prophet
 *        History:   Created by housir
 *
 * =====================================================================================
 */

#ifndef __INCLUDE__H__
#define __INCLUDE__H__
/*housir: 全局需要包含的头文件 */

/*housir: 用于在应用层和内核之间传递采集到的温度和电压值*/
typedef struct lct2991_v_temp_data_tag{
    uint8_t sign ;   /*housir: 符号位 */ 
    int hvtemp ;     /*housir: 温度/电压的整数位 8位*/
    int lvtemp ;     /*housir: 温度/电压的小数位 8位*/    
 }st_sensorinfo,*pst_sensorinfo;

/*FPGA控制命令结构体*/
typedef struct FPGA_CONTROL_CMD_ST
{
    unsigned short usRegAddr; /*寄存器地址*/
    unsigned char ucReseved; /*预留*/
    unsigned char  ucCmd; /*cmd 0:读取1:写寄存器*/ 
}FPGA_CONTROL_CMD;

/*housir: 宏定义 */
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

/*housir: 宏 */
#define NEGATIVE       1            /*  负值*/
#define POSITIVE       0            /* 正值 */

#define TOTAL_SENSOES            10/*一共10个传感器采集点*/
#define SENSOR_TEMP_TOTAL           4            /*4个温度采集点  */
#define SENSOR_V_MAX_NUM       6            /*传感器中测的电压的个数  */

#endif //__INCLUDE__H__
