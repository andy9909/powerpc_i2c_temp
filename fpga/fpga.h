#ifndef __FPGA__
#define __FPGA__

/*宏定义*/
#define FPGA_WRITE 1
#define FPGA_READ 0

/*FPGA控制命令结构体*/
typedef struct FPGA_CONTROL_CMD_ST
{
	unsigned short usRegAddr; /*寄存器地址*/
	unsigned char ucReseved; /*预留*/
	unsigned char  ucCmd; /*cmd 0:读取1:写寄存器*/ 
}FPGA_CONTROL_CMD;

#endif
