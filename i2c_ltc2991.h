/*
 * =====================================================================================
 *       Copyright (c), 2013-2020, Prophet C&S.
 *       Filename:  i2c_ltc2991.h
 *
 *    Description:  温控制器lct2991的底层调用
 *         Others:  对slave设备的读和写
 *
 *        Version:  1.0
 *        Date:  Monday, November 11, 2013 01:58:47 HKT
 *       Revision:  none
 *       Compiler:  PowerPc-gcc
 *
 *         Author:  housir houwentaoff@gmail.com
 *   Organization:  Prophet
 *
 * =====================================================================================
 */
#ifndef __I2C_LTC2991__H__
#define __I2C_LTC2991__H__

#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/hwmon-sysfs.h>
#include <linux/hwmon.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/types.h>



/**
 * @brief Read a byte, store in "value".
 *
 * @param address 7-bit I2C address 
 * @param command byte
 * @param value Byte to be read
 *
 * @return 0 on success, -1 on failure 
 */
int8_t i2c_read_byte_data(uint8_t address, uint8_t command, uint8_t *value);


/**
 * @brief Read a 16-bit word of data from register specified by "command". 
 *
 * @param address 	7-bit I2C address 
 * @param command   Command byte 
 * @param value     Word to be read 
 *
 * @return 0 on success, -1 on failure 
 */
int8_t i2c_read_word_data(uint8_t address, uint8_t command, uint16_t *value);


/**
 * @brief Write a byte of data to register specified by "command"
 *
 * @param address 7位I2C 器件的地址
 * @param command 	Command byte 
 * @param value   Byte to be written
 *
 * @return 成功返回0 失败返回-1
 */
int8_t i2c_write_byte_data(uint8_t address, uint8_t command, uint8_t value);

/**
 * @brief 写一个字16位，ADC中寄存器的读取需要
 *
 * @param address  7位I2C 器件的地址
 * @param command  Command byte 
 * @param value    Word to be written
 *
 * @return  成功返回0 失败返回-1
 */
int8_t i2c_write_word_data(uint8_t address, uint8_t command, uint16_t value);

#endif // I2C_LTC2991_H
