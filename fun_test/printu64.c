/*
 * =====================================================================================
 *       Copyright (c), 2013-2020, Prophet C&S.
 *       Filename:  printu64.c
 *
 *    Description:  打印 u64
 *         Others:
 *   
 *        Version:  1.0
 *        Date:  2013/12/31 11:28:18
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
    long long apple=0x1122334455667788;
    unsigned long long pear=0x2233445566778899;
    printf("%llx\n", pear);

    apple = ((long long )1)<<32;

    printf("%llx\n", apple);

    return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
