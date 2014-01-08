/*
 * =====================================================================================
 *       Copyright (c), 2013-2020, Prophet C&S.
 *       Filename:  kuo_hao_biaodashi.c
 *
 *    Description:  大括号也有表达式? 返回后一个表达式的值 和逗号表达式一样
 *         Others:
 *   
 *        Version:  1.0
 *        Date:  2013/12/26 13:47:42
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

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int main ( int argc, char *argv[] )
{
    int i=0;
    int m,n;
    long long mm;

    i =(
       {
            m=1;
            n=5;
       }
       );



    printf("i:%d\n", i);
    printf("sizeof: long long %d \n", sizeof(long long));

    return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
