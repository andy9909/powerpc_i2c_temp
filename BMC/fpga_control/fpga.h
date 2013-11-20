#ifndef __FPGA__
#define __FPGA__

/*�궨��*/
#define FPGA_WRITE 1
#define FPGA_READ 0

/*FPGA��������ṹ��*/
typedef struct FPGA_CONTROL_CMD_ST
{
	unsigned short usRegAddr; /*�Ĵ�����ַ*/
	unsigned char ucReseved; /*Ԥ��*/
	unsigned char  ucCmd; /*cmd 0:��ȡ1:д�Ĵ���*/ 
}FPGA_CONTROL_CMD;

#endif
