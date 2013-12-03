/*
 * =====================================================================================
 *       Copyright (c), 2013-2020, Prophet C&S.
 *       Filename:  vpx3_prt_include.h
 *
 *    Description:  需定义DBG__USER  和 DBG__KERNEL才能分别打印 和相关模块的宏
 *         Others:  还需定义当前的打印等级变量
 *   
 *        Version:  1.0
 *        Date:  Monday, November 25, 2013 03:30:36 HKT
 *       Revision:  none
 *       Compiler:  powerpc-gcc
 *
 *         Author:  housir , houwentaoff@gmail.com
 *   Organization:  Prophet
 *        History:   Created by housir
 *
 * =====================================================================================
 */
#ifndef __VPX3_PRT_INCLUDE_H
#define __VPX3_PRT_INCLUDE_H

#define DBG__KERNEL
/*housir: 驱动调试开关 */
/*内核回显调用方式*/
/*
DBGPRINT(RT_DEBUG_TRACE,("ioctl SIOCSIWGENIE length=%d, pointer=%x\n", wrqin->u.data.length, wrqin->u.data.pointer));
*/

#define KI2C_DRIVER_DEBUG               0            /*  */
#define KTEMP_V_COLLECTION_DEBUG        0            /*  */
#define KFPGA_DEBUG                     0   /*  */


#define KI2C_DRIVER_ERROR               1            /*  */
#define KTEMP_V_COLLECTION_ERROR        1            /*  */
#define KFPGA_ERROR                     1   /*  */


/*housir: 应用层调试开关 */
#define UIBMC_DEBUG                     1            /*  */



#define UIBMC_ERROR          1            /*  */
/* */
/*  Debug information verbosity: lower values indicate higher urgency */
/* */
#define K_DEBUG_OFF        0
#define K_DEBUG_ERROR      1
#define K_DEBUG_WARN       2
#define K_DEBUG_TRACE      3
#define K_DEBUG_INFO       4
#define K_DEBUG_LOUD       5

/*housir: 应用层dbug模式 */
#ifdef DBG__USER

#define __USER_TRACE_PRINT(fmt, args...) \
do\
{\
    /*printf("\ndebug in %s %s %d \n", __FILE__, __FUNCTION__, __LINE__);*/\
    printf(fmt, ##args);\
}while(0)

#define __USER_TRACE_PRINT_ERR(fmt, args...) \
do\
{\
    printf("\033[41;32m \033[05m [Error] \033[0m : %s -->%s -->%d : ", __FILE__, __FUNCTION__, __LINE__);\
    printf(fmt, ##args);\
}while(0)

#endif//DBG_USER

/*housir: 内核DBG模式 */
/*housir: 示例:KDBGPRINT(K_DEBUG_TRACE,("ioctl SIOCSIWGENIE length=%d, pointer=%x\n", wrqin->u.data.length, wrqin->u.data.pointer));
 * 需定义 unsigned long KDebugLevel = K_DEBUG_ERROR;
 */
#ifdef DBG__KERNEL
extern unsigned long KDebugLevel;
     
#define __KERNEL_TRACE_RAW(Level, Fmt)    \
do{                                   \
    if (Level <= KDebugLevel)      \
    {                               \
        printk Fmt;               \
    }                               \
}while(0)
     
#define KDBGPRINT(Level, Fmt)   __KERNEL_TRACE_RAW(Level, Fmt)
     
#define __KERNEL_TRACE_ERR(Fmt)           \
{                                   \
    printk("ERROR! ");          \
    printk Fmt;                  \
}    
     
    
#else
#define KDBGPRINT(Level, Fmt)
#define __KERNEL_TRACE_RAW(Level, Fmt)
#define __KERNEL_TRACE_ERR(Fmt)
#endif

/*housir: 驱动层 */
/*housir: 调试回显 */
#if KI2C_DRIVER_DEBUG
#define PRT_KI2C_DRIVER_DEBUG     KDBGPRINT
#else
#define PRT_KI2C_DRIVER_DEBUG(fmt, args...) 
#endif

#if KTEMP_V_COLLECTION_DEBUG
#define PRT_KTEMP_V_COLLECTION_DEBUG     KDBGPRINT
#else
#define PRT_KTEMP_V_COLLECTION_DEBUG(fmt, args...) 
#endif

#if KFPGA_DEBUG
#define PRT_KFPGA_DEBUG    KDBGPRINT
#else
#define PRT_KFPGA_DEBUG(fmt, args...) 
#endif

/*housir: 错误回显 */
#if KI2C_DRIVER_ERROR
#define PRT_KI2C_DRIVER_ERROR    {printk("[kernel module i2c_driver] "); __KERNEL_TRACE_ERR  ;}
#else
#define PRT_KI2C_DRIVER_ERROR(fmt, args...) 
#endif

#if KTEMP_V_COLLECTION_ERROR
#define PRT_KTEMP_V_COLLECTION_ERROR   {printk("[kernel module temp_v_collection] "); __KERNEL_TRACE_ERR  ;}
#else
#define PRT_KTEMP_V_COLLECTION_ERROR(fmt, args...) 
#endif

#if KFPGA_ERROR
#define PRT_KFPGA_ERROR     {printk("[kernel module fpga] "); __KERNEL_TRACE_ERR  ;}
#else
#define PRT_KFPGA_ERROR(fmt, args...) 
#endif

/*housir: 应用层 */

/*housir: 调试回显 */
#if UIBMC_DEBUG
#define PRT_IBMC_DEBUG     { printf ("[app module  ibmc] "); __USER_TRACE_PRINT  ;}
#else
#define PRT_IBMC_DEBUG(fmt, args...) 
#endif

/*housir: 错误回显 */
#if UIBMC_ERROR
#define PRT_IBMC_ERROR     { printf ("[app module ibmc] "); __USER_TRACE_PRINT_ERR  ;}
#else
#define PRT_IBMC_ERROR(fmt, args...) 
#endif


#endif//__VPX3_PRT_INCLUDE_H
