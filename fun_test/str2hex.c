/*
 * =====================================================================================
 *       Copyright (c), 2013-2020, Prophet C&S.
 *       Filename:  str2hex.c
 *
 *    Description:  测试str2hex 函数 将字符串转化为十进制数字 
 *         Others:  如0x12 为 18
 *   
 *        Version:  1.0
 *        Date:  2013/12/11 13:58:38
 *       Revision:  none
 *       Compiler:  powerpc-gcc
 *
 *         Author:  housir , houwentaoff@gmail.com
 *   Organization:  Newland
 *        History:   Created by housir
 *
 * =====================================================================================
 */

#include	<stdlib.h>


/**
 * @brief 
 *
 * @param c
 *
 * @return 
 */
static unsigned char str2hexnum(unsigned char c)
{
	if(c >= '0' && c <= '9')
		return c - '0';
	if(c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if(c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return 0; /* foo */
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  str2hex
 *  Description:  
 * =====================================================================================
 */

static unsigned long str2hex(unsigned char *str)
{
	int value = 0;
	while (*str) {
		value = value << 4;
		value |= str2hexnum(*str++);
	}

	return value;
    
    return value;
}		/* -----  end of function str2hex  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int main ( int argc, char *argv[] )
{
    char str[]="0x13";
    unsigned int uisize=0;

    uisize = str2hex(str);

    printf("%d\n", uisize);

    return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */

