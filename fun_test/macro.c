/*
 * =====================================================================================
 *       Copyright (c), 2013-2020, Prophet C&S.
 *       Filename:  macro.c
 *
 *    Description:  测试#if defined(ST_5234) 的用法
 *         Others:
 *   
 *        Version:  1.0
 *        Date:  2014/1/8 15:26:22
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
#define   ST_5234         /* 定义平台 */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  fun_son
 *  Description:  
 * =====================================================================================
 */
int fun_son()
{
    printf("ST_5234\n");
    return 1;
}		/* -----  end of function fun_son  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  fun
 *  Description:  
 * =====================================================================================
 */

int fun()
{
#if defined(ST_5234)
    return (fun_son());
#endif
}		/* -----  end of function fun  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int main ( int argc, char *argv[] )
{
    printf("%d\n",fun());
    return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */

