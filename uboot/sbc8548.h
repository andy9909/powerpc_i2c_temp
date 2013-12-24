/*
 * Copyright 2007,2009 Wind River Systems <www.windriver.com>
 * Copyright 2007 Embedded Specialties, Inc.
 * Copyright 2004, 2007 Freescale Semiconductor.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * sbc8548 board configuration file
 * Please refer to doc/README.sbc8548 for more info.
 */
#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * Top level Makefile configuration choices
 */
#ifdef CONFIG_PCI
#define CONFIG_PCI1
#endif

#ifdef CONFIG_66
#define CONFIG_SYS_CLK_DIV 1
#endif

#ifdef CONFIG_33
#define CONFIG_SYS_CLK_DIV 2
#endif

#define CONFIG_PCIE

#ifdef CONFIG_PCIE
#define CONFIG_PCIE1
#endif

/*
 * High Level Configuration Options
 */
#define CONFIG_BOOKE		1	/* BOOKE */
#define CONFIG_E500		1	/* BOOKE e500 family */
#define CONFIG_MPC85xx		1	/* MPC8540/60/55/41/48 */
#define CONFIG_MPC8548		1	/* MPC8548 specific */
#define CONFIG_SBC8548		1	/* SBC8548 board specific */

/*
 * If you want to boot from the SODIMM flash, instead of the soldered
 * on flash, set this, and change JP12, SW2:8 accordingly.
 */

/* BEGIN: Added by Niefei, 2013/6/8   问题单号:PPC Flash地址标注BOOT 地址 */
#define CONFIG_SYS_TEXT_BASE	0xfff00000
/* END:   Added by Niefei, 2013/6/8 */
#undef CONFIG_RIO

#ifdef CONFIG_PCI
#define CONFIG_FSL_PCI_INIT		/* Use common FSL init code */
#define CONFIG_SYS_PCI_64BIT    1	/* enable 64-bit PCI resources */
#endif
#ifdef CONFIG_PCIE1
#define CONFIG_FSL_PCIE_RESET   1	/* need PCIe reset errata */
#endif

#define CONFIG_TSEC_ENET		/* tsec ethernet support */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_INTERRUPTS		/* enable pci, srio, ddr interrupts */

#define CONFIG_FSL_LAW		1	/* Use common FSL init code */

/*
 * Below assumes that CCB:SYSCLK remains unchanged at 6:1 via SW2:[1-4]
 */
#ifndef CONFIG_SYS_CLK_DIV
#define CONFIG_SYS_CLK_DIV	1	/* 2, if 33MHz PCI card installed */
#endif
#define CONFIG_SYS_CLK_FREQ     99999999	

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_L2_CACHE			/* toggle L2 cache */
#define CONFIG_BTB			/* toggle branch predition */

/*
 * Only possible on E500 Version 2 or newer cores.
 */
#define CONFIG_ENABLE_36BIT_PHYS	1

#define CONFIG_BOARD_EARLY_INIT_F	1	/* Call board_pre_init */

#undef	CONFIG_SYS_DRAM_TEST			/* memory test, takes time */
#define CONFIG_SYS_MEMTEST_START	0x00200000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x00400000
/* BEGIN: Added by Niefei, 2013/6/8   问题单号:PPC REG base地址标注 */
#define CONFIG_SYS_CCSRBAR		0xe0000000
#define CONFIG_SYS_CCSRBAR_PHYS_LOW	CONFIG_SYS_CCSRBAR

/* DDR Setup */
#define CONFIG_FSL_DDR2
#undef CONFIG_FSL_DDR_INTERACTIVE
#undef CONFIG_DDR_ECC			/* only for ECC DDR module */
/*
 * A hardware errata caused the LBC SDRAM SPD and the DDR2 SPD
 * to collide, meaning you couldn't reliably read either. So
 * physically remove the LBC PC100 SDRAM module from the board
 * before enabling the two SPD options below, or check that you
 * have the hardware fix on your board via "i2c probe" and looking
 * for a device at 0x53.
 */
#undef CONFIG_SPD_EEPROM		/* Use SPD EEPROM for DDR setup */
#undef CONFIG_DDR_SPD

#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER	/* DDR controller or DMA? */
#define CONFIG_MEM_INIT_VALUE	0xDeadBeef

#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_VERY_BIG_RAM

#define CONFIG_NUM_DDR_CONTROLLERS	1
#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	2

#define CONFIG_SYS_SDRAM_SIZE	512	/* DDR is 512MB */

/*
 * Make sure required options are set
 */

#undef CONFIG_CLOCKS_IN_MHZ
/* BEGIN: Added by Niefei, 2013/6/8   问题单号:Flash base地址标注 */
#define CONFIG_SYS_BOOT_BLOCK		0xf8000000	/* start 64MB Flash */
#define CONFIG_SYS_ALT_FLASH		0xf8000000	/* 8MB soldered flash */
/* BEGIN: Added by Niefei, 2013/6/8   问题单号:PPC Flash片选0属性 */
#define CONFIG_SYS_BR0_PRELIM		0xf8001001
#define CONFIG_SYS_OR0_PRELIM		0xf8006EF7
/* END:   Added by Niefei, 2013/6/8 */
#define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_BOOT_BLOCK
#define CONFIG_SYS_FLASH_BANKS_LIST	{CONFIG_SYS_FLASH_BASE, \
					 CONFIG_SYS_ALT_FLASH}
#define CONFIG_SYS_MAX_FLASH_BANKS	1		/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	1024		/* sectors per device */
#undef	CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */

#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
/* BEGIN: Added by Niefei, 2013/6/8   问题单号:片选2属性 */
#define CONFIG_SYS_BR2_PRELIM		0xf0001801
#define CONFIG_SYS_OR2_PRELIM		0xf8006EF7
#define CONFIG_SYS_LBC_FPGA_BASE        0xf0000000
/* END:   Added by Niefei, 2013/6/8 */
/* BEGIN: Added by Niefei, 2008/6/9   问题单号:XMDEMOE升级优化 */
#define PARTION_ID_UIMAGE 0
#define PARTION_ID_ROOTFS 1
#define PARTION_ID_UBOOT 2
#define FLASH_BASE_ADDR_UIMAGE 0xffb00000
#define FALSH_ADDR_END_UIMAGE 0xffefffff
#define FLASH_BASE_ADDR_ROOTFS 0xfd000000
#define FLASH_ADDR_END_ROOTFS 0xfeffffff
#define FLASH_BASE_ADDR_UBOOT 0Xfff00000
#define FLASH_ADDR_END_UBOOT 0xffffffff
#define FLASH_BASE_ADDR_DTB 0Xffa00000
#define FLASH_ADDR_END_DTB 0xffa1ffff
#define FLASH_ERROR_ADDR 0x1
/* END:   Added by Niefei, 2008/6/9 */
/*
 * SDRAM on the Local Bus (CS3 and CS4)
 * Note that most boards have a hardware errata where both the
 * LBC SDRAM and the DDR2 SDRAM decode at 0x51, making it impossible
 * to use CONFIG_DDR_SPD unless you physically remove the LBC DIMM.
 * A hardware workaround is also available, see README.sbc8548 file.
 */

#define CONFIG_SYS_LBC_LCRR		0x00000002    /* LB clock ratio reg */
#define CONFIG_SYS_LBC_LBCR		0x00000000    /* LB config reg */
#define CONFIG_SYS_LBC_LSRT		0x20000000  /* LB sdram refresh timer */
#define CONFIG_SYS_LBC_MRTPR		0x00000000  /* LB refresh timer prescal*/

/*
 * Common settings for all Local Bus SDRAM commands.
 */
#define CONFIG_SYS_LBC_LSDMR_COMMON	( LSDMR_RFCR16		\
				| LSDMR_BSMA1516	\
				| LSDMR_PRETOACT3	\
				| LSDMR_ACTTORW3	\
				| LSDMR_BUFCMD		\
				| LSDMR_BL8		\
				| LSDMR_WRC2		\
				| LSDMR_CL3		\
				)

#define CONFIG_SYS_LBC_LSDMR_PCHALL	\
	 (CONFIG_SYS_LBC_LSDMR_COMMON | LSDMR_OP_PCHALL)
#define CONFIG_SYS_LBC_LSDMR_ARFRSH	\
	 (CONFIG_SYS_LBC_LSDMR_COMMON | LSDMR_OP_ARFRSH)
#define CONFIG_SYS_LBC_LSDMR_MRW	\
	 (CONFIG_SYS_LBC_LSDMR_COMMON | LSDMR_OP_MRW)
#define CONFIG_SYS_LBC_LSDMR_RFEN	\
	 (CONFIG_SYS_LBC_LSDMR_COMMON | LSDMR_RFEN)

#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	0xe4010000	/* Initial RAM address */
#define CONFIG_SYS_INIT_RAM_SIZE	0x4000		/* Size of used area in RAM */

#define CONFIG_SYS_INIT_L2_ADDR	0xf8f80000	/* relocate boot L2SRAM */

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*
 * For soldered on flash, (128kB/sector) we use 2 sectors for u-boot and
 * one for env+bootpg (CONFIG_SYS_TEXT_BASE=0xfffa_0000, 384kB total).  For SODIMM
 * flash (512kB/sector) we use 1 sector for u-boot, and one for env+bootpg
 * (CONFIG_SYS_TEXT_BASE=0xfff0_0000, 1MB total).  This dynamically sets the right
 * thing for MONITOR_LEN in both cases.
 */
#define CONFIG_SYS_MONITOR_LEN		(~CONFIG_SYS_TEXT_BASE + 1)
#define CONFIG_SYS_MALLOC_LEN		(1024 * 1024) /* Reserved for malloc */

/* Serial Port */
#define CONFIG_CONS_INDEX	1
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		(500000000 / CONFIG_SYS_CLK_DIV)

#define CONFIG_SYS_BAUDRATE_TABLE \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_CCSRBAR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_CCSRBAR+0x4600)

/*housir:  modified by housir*/
/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER
/*housir:  #undef CONFIG_SYS_HUSH_PARSER*/

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT		1
#define CONFIG_OF_BOARD_SETUP		1
#define CONFIG_OF_STDOUT_VIA_ALIAS	1

/*
 * I2C
 */
#define CONFIG_FSL_I2C		/* Use FSL common I2C driver */
#define CONFIG_HARD_I2C		/* I2C with hardware support*/
#undef	CONFIG_SOFT_I2C		/* I2C bit-banged */
#define CONFIG_SYS_I2C_SPEED		400000	/* I2C speed and slave address */
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50
#define CONFIG_SYS_I2C_SLAVE		0x7F
#define CONFIG_SYS_I2C_OFFSET		0x3000

/*
 * General PCI
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */
#define CONFIG_SYS_PCI_VIRT		0x80000000	/* 1G PCI TLB */
#define CONFIG_SYS_PCI_PHYS		0x80000000	/* 1G PCI TLB */

#define CONFIG_SYS_PCI1_MEM_VIRT	0x80000000
#define CONFIG_SYS_PCI1_MEM_BUS		0x80000000
#define CONFIG_SYS_PCI1_MEM_PHYS	0x80000000
#define CONFIG_SYS_PCI1_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCI1_IO_VIRT		0xe2000000
#define CONFIG_SYS_PCI1_IO_BUS		0x00000000
#define CONFIG_SYS_PCI1_IO_PHYS		0xe2000000
#define CONFIG_SYS_PCI1_IO_SIZE		0x00800000	/* 8M */

#ifdef CONFIG_PCIE1
#define CONFIG_SYS_PCIE1_MEM_VIRT	0xa0000000
#define CONFIG_SYS_PCIE1_MEM_BUS	0xa0000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0xa0000000
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE1_IO_VIRT	0xe2800000
#define CONFIG_SYS_PCIE1_IO_BUS		0x00000000
#define CONFIG_SYS_PCIE1_IO_PHYS	0xe2800000
#define CONFIG_SYS_PCIE1_IO_SIZE	0x00800000	/* 8M */
#endif

#ifdef CONFIG_RIO
/*
 * RapidIO MMU
 */
#define CONFIG_SYS_RIO_MEM_BASE	0xC0000000
#define CONFIG_SYS_RIO_MEM_SIZE	0x20000000	/* 512M */
#endif

#if defined(CONFIG_PCI)

#define CONFIG_PCI_PNP			/* do pci plug-and-play */

#undef CONFIG_EEPRO100
#undef CONFIG_TULIP

#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */

#endif	/* CONFIG_PCI */


#if defined(CONFIG_TSEC_ENET)

#define CONFIG_MII	1	/* MII PHY management */
#define CONFIG_TSEC3	1
#define CONFIG_TSEC3_NAME	"eTSEC3"
#define CONFIG_TSEC4	1
#define CONFIG_TSEC4_NAME	"eTSEC4"
#undef CONFIG_MPC85XX_FEC

#define TSEC3_PHY_ADDR		0x3
#define TSEC4_PHY_ADDR		0x4

#define TSEC3_PHYIDX	        0	
#define TSEC4_PHYIDX		1

#define TSEC3_FLAGS		TSEC_GIGABIT
#define TSEC4_FLAGS		TSEC_GIGABIT

/* Options are: eTSEC[0-3] */
#define CONFIG_ETHPRIME		"eTSEC3"
#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */
#endif	/* CONFIG_TSEC_ENET */

/*
 * Environment
 */
#define CONFIG_ENV_IS_IN_FLASH	1
/*housir: modified by housir */
/*#define CONFIG_ENV_SIZE		0x2000*/
#define CONFIG_ENV_SIZE		0x800

#if CONFIG_SYS_TEXT_BASE == 0xfff00000	/* Boot from 64MB SODIMM 小型双列直插式内存模块*/
/* BEGIN: Added by Niefei, 2013/6/7   问题单号:环境变量起始地址定义 */
/*#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + 0x80000)*/
#define CONFIG_ENV_ADDR	(FLASH_ADDR_END_DTB + 0X1)
#define CONFIG_ENV_SECT_SIZE	0x80000	/* 512K(one sector) for env */
#elif CONFIG_SYS_TEXT_BASE == 0xfffa0000	/* Boot from 8MB soldered flash */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + 0x40000)
#define CONFIG_ENV_SECT_SIZE	0x20000	/* 128K(one sector) for env */
#else
#warning undefined environment size/location.
#endif

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_PING
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_ELF
#define CONFIG_CMD_REGINFO

#if defined(CONFIG_PCI)
    #define CONFIG_CMD_PCI
#endif


#undef CONFIG_WATCHDOG			/* watchdog disabled */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_CMDLINE_EDITING			/* undef to save memory */
#define CONFIG_AUTO_COMPLETE			/* add autocompletion support */
/*housir: modified by housir 避免长文本帮助文件为U-BOOT启动命令节省空间 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory	*/
/*housir:  #undef CONFIG_SYS_LONGHELP*/

#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */
/*modify by niefei */
/* BEGIN: Added by Niefei, 2013/6/7   问题单号:为定制Boot系统命令LUSA */
#define CONFIG_SYS_PROMPT	"LUSA:=> "		/* Monitor Command Prompt */
/* END:   Added by Niefei, 2013/6/7 */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size */
#else
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
/*modified by housir CONFIG_SYS_MAXARGS*/
/*housir:  #define CONFIG_SYS_MAXARGS	16*/
#define CONFIG_SYS_MAXARGS	32		/* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size */
#define CONFIG_SYS_HZ		1000		/* decrementer freq: 1ms ticks */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(8 << 20)	/* Initial Memory map for Linux*/

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/*
 * Environment Configuration
 */

/* The mac addresses for all ethernet interface */
#if defined(CONFIG_TSEC_ENET)
#define CONFIG_HAS_ETH0
#define CONFIG_ETHADDR	 02:E0:0C:00:00:FD
#define CONFIG_HAS_ETH1
#define CONFIG_ETH1ADDR	 02:E0:0C:00:01:FD
#endif

#define CONFIG_IPADDR	 192.168.3.55

#define CONFIG_HOSTNAME	 sbc8548
#define CONFIG_ROOTPATH	 "/opt/eldk/ppc_85xx"
#define CONFIG_BOOTFILE	 "/uImage"
#define CONFIG_UBOOTPATH /u-boot.bin	/* TFTP server */

#define CONFIG_SERVERIP	 192.168.3.252
#define CONFIG_GATEWAYIP 192.168.3.1
#define CONFIG_NETMASK	 255.255.255.0

#define CONFIG_LOADADDR	1000000	/*default location for tftp and bootm*/

#define CONFIG_BOOTDELAY 3	/* -1 disables auto-boot */
#undef	CONFIG_BOOTARGS		/* the boot command will set bootargs*/
/* BEGIN: Added by Niefei, 2008/6/8   问题单号:modify baudrate 115200 */
#define CONFIG_BAUDRATE	115200
/* END:   Added by Niefei, 2008/6/8 */
#define	CONFIG_EXTRA_ENV_SETTINGS				\
 "netdev=eth0\0"						\
 "uboot=" MK_STR(CONFIG_UBOOTPATH) "\0"				\
 "tftpflash=tftpboot $loadaddr $uboot; "			\
	"protect off " MK_STR(CONFIG_SYS_TEXT_BASE) " +$filesize; "	\
	"erase " MK_STR(CONFIG_SYS_TEXT_BASE) " +$filesize; "		\
	"cp.b $loadaddr " MK_STR(CONFIG_SYS_TEXT_BASE) " $filesize; "	\
	"protect on " MK_STR(CONFIG_SYS_TEXT_BASE) " +$filesize; "		\
	"cmp.b $loadaddr " MK_STR(CONFIG_SYS_TEXT_BASE) " $filesize\0"	\
 "consoledev=ttyS0\0"				\
 "ramdiskaddr=2000000\0"			\
 "ramdiskfile=rootfs.ext2.gz\0"			\
 "fdtaddr=c00000\0"				\
 "fdtfile=sbc8548.dtb\0"

#define CONFIG_NFSBOOTCOMMAND						\
   "setenv bootargs root=/dev/nfs rw "					\
      "nfsroot=$serverip:$rootpath "					\
      "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
      "console=$consoledev,$baudrate $othbootargs;"			\
   "tftp $loadaddr $bootfile;"						\
   "tftp $fdtaddr $fdtfile;"						\
   "bootm $loadaddr - $fdtaddr"


#define CONFIG_RAMBOOTCOMMAND \
   "setenv bootargs root=/dev/ram rw "					\
      "console=$consoledev,$baudrate $othbootargs;"			\
   "tftp $ramdiskaddr $ramdiskfile;"					\
   "tftp $loadaddr $bootfile;"						\
   "tftp $fdtaddr $fdtfile;"						\
   "bootm $loadaddr $ramdiskaddr $fdtaddr"

#define CONFIG_BOOTCOMMAND	CONFIG_RAMBOOTCOMMAND

#endif	/* __CONFIG_H */
