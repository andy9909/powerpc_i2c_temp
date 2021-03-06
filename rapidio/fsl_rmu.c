/*
 * Freescale MPC85xx/MPC86xx RapidIO RMU support
 *
 * Copyright 2009 Sysgo AG
 * Thomas Moll <thomas.moll@sysgo.com>
 * - fixed maintenance access routines, check for aligned access
 *
 * Copyright 2009 Integrated Device Technology, Inc.
 * Alex Bounine <alexandre.bounine@idt.com>
 * - Added Port-Write message handling
 * - Added Machine Check exception handling
 *
 * Copyright (C) 2007, 2008, 2010, 2011 Freescale Semiconductor, Inc.
 * Zhang Wei <wei.zhang@freescale.com>
 * Lian Minghuan-B31939 <Minghuan.Lian@freescale.com>
 * Liu Gang <Gang.Liu@freescale.com>
 *
 * Copyright 2005 MontaVista Software, Inc.
 * Matt Porter <mporter@kernel.crashing.org>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/of_platform.h>
#include <linux/slab.h>

#include "fsl_rio.h"

/*housir: added by housir 用于主动测试时间 */
//#define COM_MSG_DOORBELL_TIME
//#define CLIENT_MSG_DOORBELL_TIME
/* BEGIN: Added by niefei, 2013/12/6   问题单号:修改MSG发送流程 */
extern void InboundRecvMsgPro(struct rio_mport *mport);
/* END:   Added by niefei, 2013/12/6 */
#define GET_RMM_HANDLE(mport) \
		(((struct rio_priv *)(mport->priv))->rmm_handle)

/* RapidIO definition irq, which read from OF-tree */
#define IRQ_RIO_PW(m)		(((struct fsl_rio_pw *)(m))->pwirq)
#define IRQ_RIO_BELL(m) (((struct fsl_rio_dbell *)(m))->bellirq)
#define IRQ_RIO_TX(m) (((struct fsl_rmu *)(GET_RMM_HANDLE(m)))->txirq)
#define IRQ_RIO_RX(m) (((struct fsl_rmu *)(GET_RMM_HANDLE(m)))->rxirq)

#define RIO_MIN_TX_RING_SIZE	2
#define RIO_MAX_TX_RING_SIZE	2048
#define RIO_MIN_RX_RING_SIZE	2
#define RIO_MAX_RX_RING_SIZE	2048

#define RIO_IPWMR_SEN		0x00100000
#define RIO_IPWMR_QFIE		0x00000100
#define RIO_IPWMR_EIE		0x00000020
#define RIO_IPWMR_CQ		0x00000002
#define RIO_IPWMR_PWE		0x00000001

#define RIO_IPWSR_QF		0x00100000
#define RIO_IPWSR_TE		0x00000080
#define RIO_IPWSR_QFI		0x00000010
#define RIO_IPWSR_PWD		0x00000008
#define RIO_IPWSR_PWB		0x00000004

#define RIO_EPWISR		0x10010
/* EPWISR Error match value */
#define RIO_EPWISR_PINT1	0x80000000
#define RIO_EPWISR_PINT2	0x40000000
#define RIO_EPWISR_MU		0x00000002
#define RIO_EPWISR_PW		0x00000001

#define IPWSR_CLEAR		0x98
#define OMSR_CLEAR		0x1cb3
#define IMSR_CLEAR		0x491
#define IDSR_CLEAR		0x91
#define ODSR_CLEAR		0x1c00
#define LTLEECSR_ENABLE_ALL	0xFFC000FC
#define RIO_LTLEECSR		0x060c

#define RIO_IM0SR		0x64
#define RIO_IM1SR		0x164
#define RIO_OM0SR		0x4
#define RIO_OM1SR		0x104

#define RIO_DBELL_WIN_SIZE	0x1000

#define RIO_MSG_OMR_MUI		0x00000002
#define RIO_MSG_OSR_TE		0x00000080
#define RIO_MSG_OSR_QOI		0x00000020
#define RIO_MSG_OSR_QFI		0x00000010
#define RIO_MSG_OSR_MUB		0x00000004
#define RIO_MSG_OSR_EOMI	0x00000002
#define RIO_MSG_OSR_QEI		0x00000001

#define RIO_MSG_IMR_MI		0x00000002
#define RIO_MSG_ISR_TE		0x00000080
#define RIO_MSG_ISR_QFI		0x00000010
#define RIO_MSG_ISR_DIQI	0x00000001

#define RIO_MSG_DESC_SIZE	32
#define RIO_MSG_BUFFER_SIZE	4096

#define DOORBELL_DMR_DI		0x00000002
#define DOORBELL_DSR_TE		0x00000080
#define DOORBELL_DSR_QFI	0x00000010
#define DOORBELL_DSR_DIQI	0x00000001

#define DOORBELL_MESSAGE_SIZE	0x08

struct rio_msg_regs {
	u32 omr;
	u32 osr;
	u32 pad1;
	u32 odqdpar;
	u32 pad2;
	u32 osar;
	u32 odpr;
	u32 odatr;
	u32 odcr;
	u32 pad3;
	u32 odqepar;
	u32 pad4[13];
	u32 imr;
	u32 isr;
	u32 pad5;
	u32 ifqdpar;
	u32 pad6;
	u32 ifqepar;
};

struct rio_dbell_regs {
	u32 odmr;
	u32 odsr;
	u32 pad1[4];
	u32 oddpr;
	u32 oddatr;
	u32 pad2[3];
	u32 odretcr;
	u32 pad3[12];
	u32 dmr;
	u32 dsr;
	u32 pad4;
	u32 dqdpar;
	u32 pad5;
	u32 dqepar;
};

struct rio_pw_regs {
	u32 pwmr;
	u32 pwsr;
	u32 epwqbar;
	u32 pwqbar;
};


struct rio_tx_desc {
	u32 pad1;
	u32 saddr;
	u32 dport;
	u32 dattr;
	u32 pad2;
	u32 pad3;
	u32 dwcnt;
	u32 pad4;
};

struct rio_msg_tx_ring {
	void *virt;
	dma_addr_t phys;
	void *virt_buffer[RIO_MAX_TX_RING_SIZE];
	dma_addr_t phys_buffer[RIO_MAX_TX_RING_SIZE];
	int tx_slot;
	int size;
	void *dev_id;
};

struct rio_msg_rx_ring {
	void *virt;
	dma_addr_t phys;
	void *virt_buffer[RIO_MAX_RX_RING_SIZE];
	int rx_slot;
	int size;
	void *dev_id;
};

struct fsl_rmu {
	struct rio_msg_regs __iomem *msg_regs;
	struct rio_msg_tx_ring msg_tx_ring;
	struct rio_msg_rx_ring msg_rx_ring;
	int txirq;
	int rxirq;
};

struct rio_dbell_msg {
	u16 pad1;
	u16 tid;
	u16 sid;
	u16 info;
};
/*housir: 全部变量 */
extern struct rio_mport *mem_mport;

#ifdef COM_MSG_DOORBELL_TIME 

static struct timeval gtbegin,gtend;

#endif
/**
 * fsl_rio_tx_handler - MPC85xx outbound message interrupt handler
 * @irq: Linux interrupt number
 * @dev_instance: Pointer to interrupt-specific data
 *
 * Handles outbound message interrupts. Executes a register outbound
 * mailbox event handler and acks the interrupt occurrence.
 */
static irqreturn_t
fsl_rio_tx_handler(int irq, void *dev_instance)
{
	int osr;
	struct rio_mport *port = (struct rio_mport *)dev_instance;
	struct fsl_rmu *rmu = GET_RMM_HANDLE(port);

//    printk("===>[%s]\n", __func__);

	osr = in_be32(&rmu->msg_regs->osr);

/*如果传输错误，清除传输错误寄存器*/
	if (osr & RIO_MSG_OSR_TE) {
		pr_info("RIO: outbound message transmission error\n");
		out_be32(&rmu->msg_regs->osr, RIO_MSG_OSR_TE);
		goto out;
	}
/*溢出，清除溢出位*/
	if (osr & RIO_MSG_OSR_QOI) {
		pr_info("RIO: outbound message queue overflow\n");
		out_be32(&rmu->msg_regs->osr, RIO_MSG_OSR_QOI);
		goto out;
	}

	if (osr & RIO_MSG_OSR_EOMI) {
		u32 dqp = in_be32(&rmu->msg_regs->odqdpar);
		int slot = (dqp - rmu->msg_tx_ring.phys) >> 5;
		if (port->outb_msg[0].mcback != NULL) {
			port->outb_msg[0].mcback(port, rmu->msg_tx_ring.dev_id,
					-1,
					slot);
		}
		/* Ack the end-of-message interrupt */
		out_be32(&rmu->msg_regs->osr, RIO_MSG_OSR_EOMI);
	}

#ifdef COM_MSG_DOORBELL_TIME
   do_gettimeofday(&gtbegin);
#endif


out:
	return IRQ_HANDLED;
}
#if 0
static irqreturn_t
fsl_rio_rx_handler(int irq, void *dev_instance)
{
	int isr;
	int uiRecvAddr;
	u32 uiDlyTime = 0;
	u32 uiQueue,uiDeQueue,uiVal;
	struct rio_mport *port = (struct rio_mport *)dev_instance;
	struct fsl_rmu *rmu = GET_RMM_HANDLE(port);

	uiRecvAddr = in_be32(&rmu->msg_regs->ifqdpar);
	isr = in_be32(&rmu->msg_regs->isr);
	out_be32(&rmu->msg_regs->isr, isr);
	uiQueue = in_be32(&rmu->msg_regs->ifqdpar);
	uiDeQueue = in_be32(&rmu->msg_regs->ifqepar);
	
	while(uiQueue != uiDeQueue)
	{
        uiVal = in_be32(&rmu->msg_regs->isr);
        if((uiVal >> 2) & 0x01)
        {
               printk("msg unit busy....");
            break;
        }
        uiVal = in_be32(&rmu->msg_regs->imr);
        uiVal |= 2;
        out_be32(&rmu->msg_regs->imr,uiVal);
        uiVal = in_be32(&rmu->msg_regs->imr);
        while((uiVal & 2) != 0)
        {
            if(0x10000 == uiDlyTime)
            {
                break;
            }
            uiDlyTime ++;
        }
		uiQueue = in_be32(&rmu->msg_regs->ifqdpar);
		uiDeQueue = in_be32(&rmu->msg_regs->ifqepar);

    }
	return IRQ_HANDLED;
}

#else
/**
 * fsl_rio_rx_handler - MPC85xx inbound message interrupt handler
 * @irq: Linux interrupt number
 * @dev_instance: Pointer to interrupt-specific data
 *
 * Handles inbound message interrupts. Executes a registered inbound
 * mailbox event handler and acks the interrupt occurrence.
 */
static irqreturn_t
fsl_rio_rx_handler(int irq, void *dev_instance)
{
	int isr;
	struct rio_mport *port = (struct rio_mport *)dev_instance;
	struct fsl_rmu *rmu = GET_RMM_HANDLE(port);

//    printk("housir : msg  rx_handler: deviceid id 0x%x\n",port->host_deviceid);

	isr = in_be32(&rmu->msg_regs->isr);

	if (isr & RIO_MSG_ISR_TE) {
		pr_info("RIO: inbound message reception error\n");
		out_be32((void *)&rmu->msg_regs->isr, RIO_MSG_ISR_TE);
		goto out;
	}
	//printk("disable interruput\n");
	//out_be32((void *)&rmu->msg_regs->imr, in_be32(&rmu->msg_regs->isr) & 0xffffffbf);
	/*end by niefei*/
    /* BEGIN: Added by niefei, 2013/12/5   问题单号:修改MSG接收流程 */
    InboundRecvMsgPro(port);
    /* END:   Added by niefei, 2013/12/5 */
	/* XXX Need to check/dispatch until queue empty */
	if (isr & RIO_MSG_ISR_DIQI) {
		/*
		* Can receive messages for any mailbox/letter to that
		* mailbox destination. So, make the callback with an
		* unknown/invalid mailbox number argument.
		*/
		if (port->inb_msg[0].mcback != NULL)
			port->inb_msg[0].mcback(port, rmu->msg_rx_ring.dev_id,
				-1,
				-1);

		/* Ack the queueing interrupt */
//		printk("rx_handle isr = 0x%x\n",isr);
		out_be32(&rmu->msg_regs->isr, RIO_MSG_ISR_DIQI);
		isr = in_be32(&rmu->msg_regs->isr);
		while(in_be32(&rmu->msg_regs->isr) & RIO_MSG_ISR_DIQI)
		{
			out_be32(&rmu->msg_regs->isr, RIO_MSG_ISR_DIQI);
			printk("clear isr  fail = 0x%x\n",isr);
		}
//		printk("clear isr  = 0x%x\n",isr);
	}
#ifdef  CLIENT_MSG_DOORBELL_TIME
        fsl_rio_doorbell_send(mem_mport, 0, 0x1, 0x4444);
#endif
#if 0	
	/*
	 * Configure inbound message unit:
	 *      Snooping
	 *      4KB max message size
	 *      Unmask all interrupt sources
	 *      Disable
	 */
	out_be32(&rmu->msg_regs->imr, 0x001b0060);

	/* Set number of queue entries */
	setbits32(&rmu->msg_regs->imr, (get_bitmask_order(8) - 2) << 12);

	/* Now enable the unit */
	setbits32(&rmu->msg_regs->imr, 0x1);
//end by niefei
#endif
out:
	return IRQ_HANDLED;
}
#endif
/**
 * fsl_rio_dbell_handler - MPC85xx doorbell interrupt handler
 * @irq: Linux interrupt number
 * @dev_instance: Pointer to interrupt-specific data
 *
 * Handles doorbell interrupts. Parses a list of registered
 * doorbell event handlers and executes a matching event handler.
 */
static irqreturn_t
fsl_rio_dbell_handler(int irq, void *dev_instance)
{
	int dsr;
	struct fsl_rio_dbell *fsl_dbell = (struct fsl_rio_dbell *)dev_instance;
	int i;
    struct rio_dbell_msg *dmsg_1;

	dsr = in_be32(&fsl_dbell->dbell_regs->dsr);

#ifdef COM_MSG_DOORBELL_TIME
   do_gettimeofday(&gtend);
   printk("housir:gtend:[%ld] gtbegin:[%ld] timeout %ld us\n", (long)gtend.tv_usec, gtbegin.tv_usec, gtend.tv_usec - gtbegin.tv_usec);

#endif


//	pr_info("niefei RIO: doorbell reception ok\n");
	if (dsr & DOORBELL_DSR_TE) {
		pr_info("RIO: doorbell reception error\n");
		out_be32(&fsl_dbell->dbell_regs->dsr, DOORBELL_DSR_TE);
		goto out;
	}

	if (dsr & DOORBELL_DSR_QFI) {
		pr_info("RIO: doorbell queue full\n");
		out_be32(&fsl_dbell->dbell_regs->dsr, DOORBELL_DSR_QFI);
	}

		dmsg_1 = fsl_dbell->dbell_ring.virt +
			(in_be32(&fsl_dbell->dbell_regs->dqdpar) & 0xfff);

//		printk("time _RIO: processing doorbell,"
//			" sid %2.2x tid %2.2x info %4.4x\n",
//			dmsg_1->sid, dmsg_1->tid, dmsg_1->info);



	/* XXX Need to check/dispatch until queue empty */
	if (dsr & DOORBELL_DSR_DIQI) {
		struct rio_dbell_msg *dmsg =
			fsl_dbell->dbell_ring.virt +
			(in_be32(&fsl_dbell->dbell_regs->dqdpar) & 0xfff);
		struct rio_dbell *dbell;
		int found = 0;
//        printk("1111111111111111111\n");
		pr_debug
			("RIO: processing doorbell,"
			" sid %2.2x tid %2.2x info %4.4x\n",
			dmsg->sid, dmsg->tid, dmsg->info);

		for (i = 0; i < MAX_PORT_NUM; i++) {
			if (fsl_dbell->mport[i]) {
				list_for_each_entry(dbell,
					&fsl_dbell->mport[i]->dbells, node) {
					if ((dbell->res->start
						<= dmsg->info)
						&& (dbell->res->end
						>= dmsg->info)) {
						found = 1;
						break;
					}
				}
				if (found && dbell->dinb) {
					dbell->dinb(fsl_dbell->mport[i],
						dbell->dev_id, dmsg->sid,
						dmsg->tid,
						dmsg->info);
					break;
				}
			}
		}

		if (!found) {
			pr_debug
				("RIO: spurious doorbell,"
				" sid %2.2x tid %2.2x info %4.4x\n",
				dmsg->sid, dmsg->tid,
				dmsg->info);
		}
		setbits32(&fsl_dbell->dbell_regs->dmr, DOORBELL_DMR_DI);
		out_be32(&fsl_dbell->dbell_regs->dsr, DOORBELL_DSR_DIQI);
	}

#ifdef  CLIENT_MSG_DOORBELL_TIME
        fsl_rio_doorbell_send(mem_mport, 0, dmsg_1->sid, 0x3333);
#endif

out:
	return IRQ_HANDLED;
}

void msg_unit_error_handler(void)
{

	/*XXX: Error recovery is not implemented, we just clear errors */
	out_be32((u32 *)(rio_regs_win + RIO_LTLEDCSR), 0);

	out_be32((u32 *)(rmu_regs_win + RIO_IM0SR), IMSR_CLEAR);
	out_be32((u32 *)(rmu_regs_win + RIO_IM1SR), IMSR_CLEAR);
	out_be32((u32 *)(rmu_regs_win + RIO_OM0SR), OMSR_CLEAR);
	out_be32((u32 *)(rmu_regs_win + RIO_OM1SR), OMSR_CLEAR);

	out_be32(&dbell->dbell_regs->odsr, ODSR_CLEAR);
	out_be32(&dbell->dbell_regs->dsr, IDSR_CLEAR);

	out_be32(&pw->pw_regs->pwsr, IPWSR_CLEAR);
}

/**
 * fsl_rio_port_write_handler - MPC85xx port write interrupt handler
 * @irq: Linux interrupt number
 * @dev_instance: Pointer to interrupt-specific data
 *
 * Handles port write interrupts. Parses a list of registered
 * port write event handlers and executes a matching event handler.
 */
static irqreturn_t
fsl_rio_port_write_handler(int irq, void *dev_instance)
{
	u32 ipwmr, ipwsr;
	struct fsl_rio_pw *pw = (struct fsl_rio_pw *)dev_instance;
	u32 epwisr, tmp;

	epwisr = in_be32(rio_regs_win + RIO_EPWISR);
	if (!(epwisr & RIO_EPWISR_PW))
		goto pw_done;

	ipwmr = in_be32(&pw->pw_regs->pwmr);
	ipwsr = in_be32(&pw->pw_regs->pwsr);

#ifdef DEBUG_PW
	pr_debug("PW Int->IPWMR: 0x%08x IPWSR: 0x%08x (", ipwmr, ipwsr);
	if (ipwsr & RIO_IPWSR_QF)
		pr_debug(" QF");
	if (ipwsr & RIO_IPWSR_TE)
		pr_debug(" TE");
	if (ipwsr & RIO_IPWSR_QFI)
		pr_debug(" QFI");
	if (ipwsr & RIO_IPWSR_PWD)
		pr_debug(" PWD");
	if (ipwsr & RIO_IPWSR_PWB)
		pr_debug(" PWB");
	pr_debug(" )\n");
#endif
	/* Schedule deferred processing if PW was received */
	if (ipwsr & RIO_IPWSR_QFI) {
		/* Save PW message (if there is room in FIFO),
		 * otherwise discard it.
		 */
		if (kfifo_avail(&pw->pw_fifo) >= RIO_PW_MSG_SIZE) {
			pw->port_write_msg.msg_count++;
			kfifo_in(&pw->pw_fifo, pw->port_write_msg.virt,
				 RIO_PW_MSG_SIZE);
		} else {
			pw->port_write_msg.discard_count++;
			pr_debug("RIO: ISR Discarded Port-Write Msg(s) (%d)\n",
				 pw->port_write_msg.discard_count);
		}
		/* Clear interrupt and issue Clear Queue command. This allows
		 * another port-write to be received.
		 */
		out_be32(&pw->pw_regs->pwsr,	RIO_IPWSR_QFI);
		out_be32(&pw->pw_regs->pwmr, ipwmr | RIO_IPWMR_CQ);

		schedule_work(&pw->pw_work);
	}

	if ((ipwmr & RIO_IPWMR_EIE) && (ipwsr & RIO_IPWSR_TE)) {
		pw->port_write_msg.err_count++;
		pr_debug("RIO: Port-Write Transaction Err (%d)\n",
			 pw->port_write_msg.err_count);
		/* Clear Transaction Error: port-write controller should be
		 * disabled when clearing this error
		 */
		out_be32(&pw->pw_regs->pwmr, ipwmr & ~RIO_IPWMR_PWE);
		out_be32(&pw->pw_regs->pwsr,	RIO_IPWSR_TE);
		out_be32(&pw->pw_regs->pwmr, ipwmr);
	}

	if (ipwsr & RIO_IPWSR_PWD) {
		pw->port_write_msg.discard_count++;
		pr_debug("RIO: Port Discarded Port-Write Msg(s) (%d)\n",
			 pw->port_write_msg.discard_count);
		out_be32(&pw->pw_regs->pwsr, RIO_IPWSR_PWD);
	}

pw_done:
	if (epwisr & RIO_EPWISR_PINT1) {
		tmp = in_be32(rio_regs_win + RIO_LTLEDCSR);
		pr_debug("RIO_LTLEDCSR = 0x%x\n", tmp);
		fsl_rio_port_error_handler(0);
	}

	if (epwisr & RIO_EPWISR_PINT2) {
		tmp = in_be32(rio_regs_win + RIO_LTLEDCSR);
		pr_debug("RIO_LTLEDCSR = 0x%x\n", tmp);
		fsl_rio_port_error_handler(1);
	}

	if (epwisr & RIO_EPWISR_MU) {
		tmp = in_be32(rio_regs_win + RIO_LTLEDCSR);
		pr_debug("RIO_LTLEDCSR = 0x%x\n", tmp);
		msg_unit_error_handler();
	}

	return IRQ_HANDLED;
}

static void fsl_pw_dpc(struct work_struct *work)
{
	struct fsl_rio_pw *pw = container_of(work, struct fsl_rio_pw, pw_work);
	u32 msg_buffer[RIO_PW_MSG_SIZE/sizeof(u32)];

	/*
	 * Process port-write messages
	 */
	while (kfifo_out_spinlocked(&pw->pw_fifo, (unsigned char *)msg_buffer,
			 RIO_PW_MSG_SIZE, &pw->pw_fifo_lock)) {
		/* Process one message */
#ifdef DEBUG_PW
		{
		u32 i;
		pr_debug("%s : Port-Write Message:", __func__);
		for (i = 0; i < RIO_PW_MSG_SIZE/sizeof(u32); i++) {
			if ((i%4) == 0)
				pr_debug("\n0x%02x: 0x%08x", i*4,
					 msg_buffer[i]);
			else
				pr_debug(" 0x%08x", msg_buffer[i]);
		}
		pr_debug("\n");
		}
#endif
		/* Pass the port-write message to RIO core for processing */
		rio_inb_pwrite_handler((union rio_pw_msg *)msg_buffer);
	}
}

/**
 * fsl_rio_pw_enable - enable/disable port-write interface init
 * @mport: Master port implementing the port write unit
 * @enable:    1=enable; 0=disable port-write message handling
 */
int fsl_rio_pw_enable(struct rio_mport *mport, int enable)
{
	u32 rval;

	rval = in_be32(&pw->pw_regs->pwmr);

	if (enable)
		rval |= RIO_IPWMR_PWE;
	else
		rval &= ~RIO_IPWMR_PWE;

	out_be32(&pw->pw_regs->pwmr, rval);

	return 0;
}

/**
 * fsl_rio_port_write_init - MPC85xx port write interface init
 * @mport: Master port implementing the port write unit
 *
 * Initializes port write unit hardware and DMA buffer
 * ring. Called from fsl_rio_setup(). Returns %0 on success
 * or %-ENOMEM on failure.
 */

int fsl_rio_port_write_init(struct fsl_rio_pw *pw)
{
	int rc = 0;

	/* Following configurations require a disabled port write controller */
	out_be32(&pw->pw_regs->pwmr,
		 in_be32(&pw->pw_regs->pwmr) & ~RIO_IPWMR_PWE);

	/* Initialize port write */
	pw->port_write_msg.virt = dma_alloc_coherent(pw->dev,
					RIO_PW_MSG_SIZE,
					&pw->port_write_msg.phys, GFP_KERNEL);
	if (!pw->port_write_msg.virt) {
		pr_err("RIO: unable allocate port write queue\n");
		return -ENOMEM;
	}

	pw->port_write_msg.err_count = 0;
	pw->port_write_msg.discard_count = 0;

	/* Point dequeue/enqueue pointers at first entry */
	out_be32(&pw->pw_regs->epwqbar, 0);
	out_be32(&pw->pw_regs->pwqbar, (u32) pw->port_write_msg.phys);

	pr_debug("EIPWQBAR: 0x%08x IPWQBAR: 0x%08x\n",
		 in_be32(&pw->pw_regs->epwqbar),
		 in_be32(&pw->pw_regs->pwqbar));

	/* Clear interrupt status IPWSR */
	out_be32(&pw->pw_regs->pwsr,
		 (RIO_IPWSR_TE | RIO_IPWSR_QFI | RIO_IPWSR_PWD));

	/* Configure port write contoller for snooping enable all reporting,
	   clear queue full */
	out_be32(&pw->pw_regs->pwmr,
		 RIO_IPWMR_SEN | RIO_IPWMR_QFIE | RIO_IPWMR_EIE | RIO_IPWMR_CQ);


	/* Hook up port-write handler */
	rc = request_irq(IRQ_RIO_PW(pw), fsl_rio_port_write_handler,
			IRQF_SHARED, "port-write", (void *)pw);
	if (rc < 0) {
		pr_err("MPC85xx RIO: unable to request inbound doorbell irq");
		goto err_out;
	}
	/* Enable Error Interrupt */
	out_be32((u32 *)(rio_regs_win + RIO_LTLEECSR), LTLEECSR_ENABLE_ALL);

	INIT_WORK(&pw->pw_work, fsl_pw_dpc);
	spin_lock_init(&pw->pw_fifo_lock);
	if (kfifo_alloc(&pw->pw_fifo, RIO_PW_MSG_SIZE * 32, GFP_KERNEL)) {
		pr_err("FIFO allocation failed\n");
		rc = -ENOMEM;
		goto err_out_irq;
	}

	pr_debug("IPWMR: 0x%08x IPWSR: 0x%08x\n",
		 in_be32(&pw->pw_regs->pwmr),
		 in_be32(&pw->pw_regs->pwsr));

	return rc;

err_out_irq:
	free_irq(IRQ_RIO_PW(pw), (void *)pw);
err_out:
	dma_free_coherent(pw->dev, RIO_PW_MSG_SIZE,
		pw->port_write_msg.virt,
		pw->port_write_msg.phys);
	return rc;
}

/**
 * fsl_rio_doorbell_send - Send a MPC85xx doorbell message
 * @mport: RapidIO master port info
 * @index: ID of RapidIO interface
 * @destid: Destination ID of target device
 * @data: 16-bit info field of RapidIO doorbell message
 *
 * Sends a MPC85xx doorbell message. Returns %0 on success or
 * %-EINVAL on failure.
 */
 #if 0 //jg.xu
int fsl_rio_doorbell_send(struct rio_mport *mport,
				int index, u16 destid, u16 data)
{
	pr_debug("fsl_doorbell_send: index %d destid %4.4x data %4.4x\n",
		 index, destid, data);

	/* In the serial version silicons, such as MPC8548, MPC8641,
	 * below operations is must be.
	 */
	out_be32(&dbell->dbell_regs->odmr, 0x00000000);
	out_be32(&dbell->dbell_regs->odretcr, 0x00000004);
	out_be32(&dbell->dbell_regs->oddpr, destid << 16);
	out_be32(&dbell->dbell_regs->oddatr, (index << 20) | data);
	out_be32(&dbell->dbell_regs->odmr, 0x00000001);

	return 0;
}
 #else
 
#include <linux/delay.h>
 
 int fsl_rio_doorbell_send(struct rio_mport *mport,
				 int index, u16 destid, u16 data)
 {
	 unsigned int uiTimeOut = 0x100;
	 u32 uiVal = 0;
//	 printk("fsl_doorbell_send: index %d destid %4.4x data %4.4x\n",
//		  index, destid, data);
 
	 /* In the serial version silicons, such as MPC8548, MPC8641,
	  * below operations is must be.
	  */
//	 printk("jg odsr 1 %x,%p\n", in_be32(&dbell->dbell_regs->odsr),&dbell->dbell_regs->odsr);
	 while(uiTimeOut--)
	 {
		 uiVal = in_be32(&dbell->dbell_regs->odsr);
		 if(0 == (uiVal& 0x04))
		 {
			 uiVal = in_be32(&dbell->dbell_regs->odsr);
			 uiVal |= ((1<<1)|(1<<10)|(1<<11)|(1<<12));
			 out_be32(&dbell->dbell_regs->odsr,uiVal);
			 break;
		 }
	 }
 
//	 printk("jg uiTimeOut %x\n",uiTimeOut);
		 
	 out_be32(&dbell->dbell_regs->odmr, 0x00000000);
	 //out_be32(&dbell->dbell_regs->odretcr, 0x00000004);
	 out_be32(&dbell->dbell_regs->oddpr, destid << 16);
	 if(1){ //jg.xu
	 uiVal = in_be32(&dbell->dbell_regs->oddatr);
	 uiVal &= (~(0xffff));
	 out_be32(&dbell->dbell_regs->oddatr,uiVal);
			 
	 uiVal = in_be32(&dbell->dbell_regs->oddatr);
//	 printk("jg uiVal %x\n",uiVal);
	 uiVal |= data;
	 out_be32(&dbell->dbell_regs->oddatr,uiVal);
		 }
	 //out_be32(&dbell->dbell_regs->oddatr,(index << 20) | data);
	 out_be32(&dbell->dbell_regs->odmr, 0x00000001);

#ifdef COM_MSG_DOORBELL_TIME
   do_gettimeofday(&gtbegin);
#endif

//	 mdelay(50);//////////////////???????????????????????
//	 printk("jg odsr 2 %x\n", in_be32(&dbell->dbell_regs->odsr));
	 return 0;
 }

/**
 * @brief           向目标id发送一个门铃
 *
 * @param vdestid   RapidIO的接口设备ID
 * @param vdata     16位的消息内容
 *
 * @return          成功返回0，不成功返回非0
 */
int rio_doorbell_send(u16 vdestid, u16 vdata)
{
	u16 destid,data;

    destid = vdestid;
    data = vdata;
#if defined(SBC_8548)
	return (fsl_rio_doorbell_send(mem_mport,0,destid,data));
#endif
}

#if 0
 /*Niefei*/
  static irqreturn_t
  rioSendMsgISR(int irq, void *dev_instance)
  {
	  int osr,odatr;
	  struct rio_mport *port = (struct rio_mport *)dev_instance;
	  struct fsl_rmu *rmu = GET_RMM_HANDLE(port);
 
	 /*读取ODATR*/
	 printk("irs_proc\n");
	 odatr = in_be32(&rmu->msg_regs->odatr);
	 odatr &=(~(1<<29));
	 out_be32(&rmu->msg_regs->odatr, odatr);
	 
	 osr = in_be32(&rmu->msg_regs->osr);
	 osr |=((1<<0)|(1<<1)|(1<<4)|(1<<5)|(1<<7)|(1<<10)|(1<<11)|(1<<11));
	 out_be32(&rmu->msg_regs->odatr, osr);
 
	 odatr = in_be32(&rmu->msg_regs->odatr);
	 odatr |=(1<<29);
	 out_be32(&rmu->msg_regs->odatr, odatr);
  
	  return IRQ_HANDLED;
  }

 /*****************************************************************************
  func : endif
  description : RapidIO发送message
  input : 
  output :
  return :
  作者:聂飞
  时间:2013-11-26
 *****************************************************************************/
int OutBoundSendMsg(struct rio_mport *mport,unsigned short v_usDestId,u32 v_uMbox,void *v_pbuffer,u32 v_uiSize)
{
	 unsigned char *buffer;
	int ret= -1;
	u32 uMbox,uiSize;
	int uiRegOsr,uiRegOmr,uiRegOdatr;
	unsigned short usDestId = 0;
	u32 timeOut =0x20;
	static int suiFlag = 0;
	 struct fsl_rmu *rmu = GET_RMM_HANDLE(mport);

	 	uMbox = v_uMbox;
	usDestId = v_usDestId;
	uiSize = v_uiSize;
	buffer = (unsigned char*)v_pbuffer; 
	/*outbound MSG init*/
	buffer = (unsigned char *)kmalloc(4096*2 + 0x100,GFP_KERNEL);
	if(NULL == buffer)
	{
		printk("kmalloc mem fail \n");
		return ret;
	}
	//buffer = (u32 *)buffer & 0xffffff00;
	memset(buffer, 0x0,4096*2);
	if(suiFlag == 0)
	{
	/*注册中断*/
		printk("request_irq = 0x%x",(int)IRQ_RIO_TX(mport));
		ret = request_irq(IRQ_RIO_TX(mport), rioSendMsgISR, 0,
		 "msg_tx", (void *)mport);
		if(ret < 0)
		{
		printk("reuest irq fail\n");
		kfree(buffer);
		return ret;
		}
		suiFlag = 1;
	}
	/*direct msg mode init*/
	while(timeOut)
	{
		uiRegOsr = in_be32(&rmu->msg_regs->osr);
		if(0x01 & uiRegOsr)
		{
			timeOut--;
		}
		else
		{
			break;
		}
	}
	if(timeOut == 0)
	{
		printk("send mesge eror  time out ");
		return -1;
	}
	uiRegOmr = in_be32(&rmu->msg_regs->omr);
	uiRegOmr &= (~(1<<0));
	out_be32(&rmu->msg_regs->omr, uiRegOmr);
	
	uiRegOsr= in_be32(&rmu->msg_regs->osr);
	uiRegOsr|= ((1<<12)|(1<<11)|(1<<10)|(1<<7)|(1<<5)|(1<<4)|(1<<1)|(1<<0));
	out_be32(&rmu->msg_regs->osr, uiRegOsr);

	uiRegOmr = in_be32(&rmu->msg_regs->omr);
	uiRegOmr |= ((1<<20)|(0x0a<<12)|(1<<2));
	out_be32(&rmu->msg_regs->omr, uiRegOmr);

	uiRegOdatr= in_be32(&rmu->msg_regs->odatr);
	uiRegOdatr |= (1<<29);
	out_be32(&rmu->msg_regs->odatr, uiRegOdatr);
	out_be32((&rmu->msg_regs->odatr) +0x10, uiRegOdatr);
/*配置并发送MSG*/
uiRegOmr = in_be32(&rmu->msg_regs->omr);
uiRegOmr  &= (~(1<<0));
out_be32(&rmu->msg_regs->omr, uiRegOmr);

	uiRegOsr= in_be32(&rmu->msg_regs->osr);
	printk("osr r=0x%x ,omr=0x%x\n ",uiRegOsr,uiRegOmr);
	if((uiRegOsr >> 20) & 0x01)
	{
		printk("mail_box  full return \n ");
		kfree(buffer);
		return -1;
	}
	uiRegOmr = in_be32(&rmu->msg_regs->omr);
	uiRegOmr  |= (1<<2);
	out_be32(&rmu->msg_regs->omr, uiRegOmr);
	printk("osar_value = 0x%x\n",((u32)buffer |0x4));
	out_be32(&rmu->msg_regs->osar, ((u32)buffer |0x4));
	out_be32(&rmu->msg_regs->odpr, (v_usDestId << 16));
	out_be32(&rmu->msg_regs->odcr, 4096);
	//omgr
	out_be32((&rmu->msg_regs->odcr) +0x10,0);
	out_be32((&rmu->msg_regs->odcr) +0x14,0);
	
	uiRegOdatr= in_be32(&rmu->msg_regs->odatr);
	uiRegOdatr  &= (~(1<<31));
	out_be32(&rmu->msg_regs->odatr, uiRegOdatr);
	/*START*/
	uiRegOmr = in_be32(&rmu->msg_regs->omr);
	printk("omr = 0x%x \n ",uiRegOmr);
	uiRegOmr  |= 0X01;
	out_be32(&rmu->msg_regs->omr, uiRegOmr);
	kfree(buffer);
	return 0;
	
}
#else
 /*****************************************************************************
  func : OutBoundSendMsg
  description : RapidIO发送message
  input : 
  output :
  return :
  作者:聂飞
  时间:2013-11-26
 *****************************************************************************/
  int OutBoundSendMsg(struct rio_mport *mport,unsigned short v_usDestId,u32 v_uMbox,void *v_pbuffer,u32 v_uiSize)
 
 {
	 u32 uMbox,uiSize,uiOpenId;
	 void *buffer;
	 unsigned short usDestId = 0;
	 static u32 uiOpenFlag = 0;

	
	 struct fsl_rmu *rmu = GET_RMM_HANDLE(mport);
	 u32 omr;
	 struct rio_tx_desc *desc = (struct rio_tx_desc *)rmu->msg_tx_ring.virt
					 + rmu->msg_tx_ring.tx_slot;
	 int ret = 0;
	 
	uMbox = v_uMbox;
	usDestId = v_usDestId;
	uiSize = v_uiSize;
	buffer = v_pbuffer; 
	
	buffer = (void *)kmalloc(0x1000,GFP_KERNEL);
	if(NULL == buffer)
	{
		printk("error kmalloc  return\n");
		return -1;
	}
	
//	 printk("RIO: fsl_add_outb_message(): destid %4.4x mbox %d buffer " 
//	  "%p len %8.8zx\n", usDestId, uMbox, buffer, uiSize);
//	 pr_debug("RIO: fsl_add_outb_message(): destid %4.4x mbox %d buffer " 
//		  "%p len %8.8zx\n", usDestId, uMbox, buffer, uiSize);
	 if ((uiSize < 8) || (uiSize > RIO_MAX_MSG_SIZE)) {
		 ret = -EINVAL;
		 goto out;
	 }
/*housir:  	 uiOpenId = 0x12;*/
	if(uiOpenFlag == 0)
	{
		fsl_open_outb_mbox(mport,(void *)&uiOpenId,0,1024);
		uiOpenFlag = 1;
	}
// 	printk("virt_buffer = %p\n",rmu->msg_tx_ring.virt_buffer[rmu->msg_tx_ring.tx_slot]);

	 /* Copy and clear rest of buffer */
	 memset(buffer, 0x1,uiSize);
	 memcpy(rmu->msg_tx_ring.virt_buffer[rmu->msg_tx_ring.tx_slot], buffer,
			 uiSize);
//	 printk("memcpy ok\n");
	 if (uiSize < (RIO_MAX_MSG_SIZE - 4))
		 memset(rmu->msg_tx_ring.virt_buffer[rmu->msg_tx_ring.tx_slot]
				 + uiSize, 0, RIO_MAX_MSG_SIZE - uiSize);
/*下面的操作是填充MSG 描述符的过程，RAPIO 在OPEN时申请了物理内存地址
这块物理内存填充为MSG描述符，odqdpar寄存器指示了当前描述符的地址
描述符中的内容RAPIDIO硬件会主动刷新到对应的寄存器中，软件不需要操作
只需要刷新MSG描述符即可*/
	 /* Set mbox field for message, and set destid */
	 desc = (struct rio_tx_desc *)rmu->msg_tx_ring.virt
					 + rmu->msg_tx_ring.tx_slot;
	 desc->dport = (usDestId << 16) | (uMbox & 0x3);
	 /* Enable EOMI interrupt and priority */
	 desc->dattr = 0x28000000 | ((mport->index) << 20);
	 /* Set transfer size aligned to next power of 2 (in double words) 计数控制寄存器，控制发送数据长度*/
	 desc->dwcnt = is_power_of_2(uiSize) ? uiSize : 1 << get_bitmask_order(uiSize);
 
	 /* Set snooping and source buffer address */
//	 printk("phy_buffer =%p",&rmu->msg_tx_ring.phys_buffer[rmu->msg_tx_ring.tx_slot]);
//	 desc->saddr = 0x00000004
//		 | rmu->msg_tx_ring.phys_buffer[rmu->msg_tx_ring.tx_slot];
	 /* Increment enqueue pointer */
	 omr = in_be32(&rmu->msg_regs->omr);
//	 printk("omr reg =%p,value = 0x%x\n",&rmu->msg_regs->omr,omr);
	 out_be32(&rmu->msg_regs->omr, omr | RIO_MSG_OMR_MUI);
/* BEGIN: Deleted by niefei, 2013/12/4   问题单号:修改MSG发送流程 */
    /*启动发送*/
	//out_be32(&rmu->msg_regs->omr, omr & 0xfffffffe);
	//out_be32(&rmu->msg_regs->omr, omr | 0x1);
	 /* Go to next descriptor */
/* END: Deleted by niefei, 2013/12/4 */
	 if (++rmu->msg_tx_ring.tx_slot == rmu->msg_tx_ring.size)
		 rmu->msg_tx_ring.tx_slot = 0;


	 #if 0
	#endif
	 out:
	 return ret;
 }
/**
 * @brief              向指定id 邮箱发送指定大小的msg
 *
 * @param usDestId    目标ID
 * @param uMbox       邮箱号
 * @param buffer      指向本地消息地址的指针
 * @param uiSize      消息长度，以字节为单位，只能在8~4096之间，2的幂次方
 *
 * @return    0:返回成功  22:参数错误         
 */
int rio_msg_send(unsigned short usDestId,u32 uMbox,void *buffer,u32 uiSize)
{
#if defined(SBC_8548)
	return (OutBoundSendMsg(mem_mport,usDestId,uMbox,buffer,uiSize));
#endif
}
/*****************************************************************************
 func : InboundMsgPro
 description : RapidIO接收message 处理
 input : 
 output :
 return :
 作者:聂飞
 时间:2013-12-6
*****************************************************************************/
void InboundRecvMsgPro(struct rio_mport *mport)
{
    void *buffer;
    u32 phys_buf;
    int buf_idx;
    unsigned char *pValue = NULL;
	struct fsl_rmu *rmu = GET_RMM_HANDLE(mport);
        /*phys_buf值为当前需要处理MSG单元存放地址*/
	phys_buf = in_be32(&rmu->msg_regs->ifqdpar);

	/* If no more messages, then bail out  ，如果当前处理单元MSG与下一个要处理单元MSG
    的地址相等，意味着没有需要处理的MSG*/
	if (phys_buf == in_be32(&rmu->msg_regs->ifqepar))
	goto out2;
/*rmu->msg_rx_ring.phys是申请存放MSG连续空间的起始地址，每个MSG占用RIO_MAX_MSG_SIZE
字节，余数运算可以计算出buf_idx，即获取第几个偏移MSG*/
	buffer = rmu->msg_rx_ring.virt + (phys_buf
					- rmu->msg_rx_ring.phys);
	buf_idx = (phys_buf - rmu->msg_rx_ring.phys) / RIO_MAX_MSG_SIZE;
    /*virt_buffer用于MSG存放，使用fsl_add_inb_buffer可以完成*/
	if (!rmu->msg_rx_ring.virt_buffer[buf_idx]) 
	{
		printk(" niefei :RIO: inbound message copy failed, no buffers\n");
		goto out1;
	}
    out1:
    pValue = (unsigned char *)rmu->msg_rx_ring.virt_buffer[buf_idx];
//    printk("recv msg  frist byte = 0x%x\n",pValue[0]);
	/*当软件接收完一帧MSG后，更新IMR的MI bit位RAPIDIO硬件使得IMFQDPAR 增加*/
	setbits32(&rmu->msg_regs->imr, RIO_MSG_IMR_MI);
//    printk("get message success ,before update IMR_MI,qdpar =0x%x,qepar=0x%x\n",in_be32(&rmu->msg_regs->ifqdpar),in_be32(&rmu->msg_regs->ifqepar));
	out2:
    return;/*housir: added by housir */
//	printk("rapiod user pro Msg buf here \n");
}
/*****************************************************************************
 func : endif
 description : RapidIO接收message
 input : 
 output :
 return :
 作者:聂飞
 时间:2013-11-26
*****************************************************************************/
int InBoundGetMsg(struct rio_mport *mport,unsigned short v_usDestId,u32 v_uMbox)
{
	u32 uMbox,uiOpenId;
	void *buffer;
	void *buf=NULL;
	u32 phys_buf;
    u32 uiEntriesNum = 8;
	int buf_idx;
	unsigned short usDestId = 0;
	static u32 uiOpenFlag = 0;
	struct fsl_rmu *rmu = GET_RMM_HANDLE(mport);
	unsigned char *pValue;
    
	uMbox = v_uMbox;
	usDestId = v_usDestId;
	uiOpenId = (u32)usDestId;
/* BEGIN: Deleted by niefei, 2013/12/4   问题单号:修改getMSG流程 */
	//buf = (void *)kmalloc(0x1000,GFP_KERNEL);
	//if(NULL == buf)
	//{
	//	printk("error get msg kalloc  return\n");
	//	return -1;
	//}
	//fsl_add_inb_buffer(mport,0,buf);
    /* END: Deleted by niefei, 2013/12/4 */
	//uiOpenId = 0x12;
	if(uiOpenFlag == 0)
	{
		fsl_open_inb_mbox(mport,(void *)&uiOpenId,0,uiEntriesNum);
//		printk("inbound get msg init\n");
        /* BEGIN: Added by niefei, 2013/12/4   问题单号:修改getMSG流程 */
        buf = (void *)kmalloc(RIO_MAX_MSG_SIZE * (uiEntriesNum +1),GFP_KERNEL);
        if(NULL == buf)
        {
            fsl_close_inb_mbox(mport,0);
            printk("error get msg kalloc  return\n");
            return -1;
        }
        while(rmu->msg_rx_ring.rx_slot < rmu->msg_rx_ring.size)
        {
//            printk("added buf for rmu->msg_rx_ring.rx_slot[%d]\n ",rmu->msg_rx_ring.rx_slot ); 
            fsl_add_inb_buffer(mport,0,(buf + rmu->msg_rx_ring.rx_slot * RIO_MAX_MSG_SIZE));
             
            if(0 == rmu->msg_rx_ring.rx_slot)
            {
                break;
            }
        }
        /* END:   Added by niefei, 2013/12/4 */
		//uiOpenFlag = 1;
		return 0;
	}
    /*phys_buf值为当前需要处理MSG单元存放地址*/
	phys_buf = in_be32(&rmu->msg_regs->ifqdpar);

	/* If no more messages, then bail out  ，如果当前处理单元MSG与下一个要处理单元MSG
    的地址相等，意味着没有需要处理的MSG*/
	if (phys_buf == in_be32(&rmu->msg_regs->ifqepar))
	goto out2;
/*rmu->msg_rx_ring.phys是申请存放MSG连续空间的起始地址，每个MSG占用RIO_MAX_MSG_SIZE
字节，余数运算可以计算出buf_idx，即获取第几个偏移MSG*/
	buffer = rmu->msg_rx_ring.virt + (phys_buf
					- rmu->msg_rx_ring.phys);
	buf_idx = (phys_buf - rmu->msg_rx_ring.phys) / RIO_MAX_MSG_SIZE;
    /*virt_buffer用于MSG存放，使用fsl_add_inb_buffer可以完成*/
	buf = rmu->msg_rx_ring.virt_buffer[buf_idx];

	if (!buf) 
	{
		printk(" niefei :RIO: inbound message copy failed, no buffers\n");
		goto out1;
	}

	/* Copy max message size, caller is expected to allocate that big */
	memcpy(buf, buffer, RIO_MAX_MSG_SIZE);
	pValue = (unsigned char *)buf;
//	printk("recv msg  addr = %p,pValue=0x%x\n",buf,pValue[0]);
	/* Clear the available buffer */
    /* BEGIN: Deleted by niefei, 2013/12/4   问题单号:修改getMSG流程 */
	//rmu->msg_rx_ring.virt_buffer[buf_idx] = NULL;
    /* END: Deleted by niefei, 2013/12/4 */
	out1:
//	printk("get message success,update IMR_MI\n");
    /*当软件接收完一帧MSG后，更新IMR的MI bit位RAPIDIO硬件使得IMFQDPAR 增加*/
	setbits32(&rmu->msg_regs->imr, RIO_MSG_IMR_MI);
    /* BEGIN: Deleted by niefei, 2013/12/4   问题单号:修改getMSG流程 */
	/*add by niefei*/
	//fsl_close_inb_mbox(mport,0);
	//uiOpenFlag=0;
    /* END: Deleted by niefei, 2013/12/4 */
	/*end */
	out2:
	printk("NO msg recived\n");
	//return buf;
	return 0;
}

#endif
 #endif

/**
 * fsl_add_outb_message - Add message to the MPC85xx outbound message queue
 * @mport: Master port with outbound message queue
 * @rdev: Target of outbound message
 * @mbox: Outbound mailbox
 * @buffer: Message to add to outbound queue
 * @len: Length of message
 *
 * Adds the @buffer message to the MPC85xx outbound message queue. Returns
 * %0 on success or %-EINVAL on failure.
 */
int
fsl_add_outb_message(struct rio_mport *mport, struct rio_dev *rdev, int mbox,
			void *buffer, size_t len)
{
	struct fsl_rmu *rmu = GET_RMM_HANDLE(mport);
	u32 omr;
	struct rio_tx_desc *desc = (struct rio_tx_desc *)rmu->msg_tx_ring.virt
					+ rmu->msg_tx_ring.tx_slot;
	int ret = 0;

//    printk("===>[%s]\n", __func__);

//	pr_debug("RIO: fsl_add_outb_message(): destid %4.4x mbox %d buffer " 
//		 "%p len %8.8zx\n", rdev->destid, mbox, buffer, len);
	if ((len < 8) || (len > RIO_MAX_MSG_SIZE)) {
		ret = -EINVAL;
		goto out;
	}

	/* Copy and clear rest of buffer */
	memcpy(rmu->msg_tx_ring.virt_buffer[rmu->msg_tx_ring.tx_slot], buffer,
			len);
	if (len < (RIO_MAX_MSG_SIZE - 4))
		memset(rmu->msg_tx_ring.virt_buffer[rmu->msg_tx_ring.tx_slot]
				+ len, 0, RIO_MAX_MSG_SIZE - len);

	/* Set mbox field for message, and set destid */
	desc->dport = (rdev->destid << 16) | (mbox & 0x3);

	/* Enable EOMI interrupt and priority */
	desc->dattr = 0x28000000 | ((mport->index) << 20);

	/* Set transfer size aligned to next power of 2 (in double words) */
	desc->dwcnt = is_power_of_2(len) ? len : 1 << get_bitmask_order(len);

	/* Set snooping and source buffer address */
	desc->saddr = 0x00000004
		| rmu->msg_tx_ring.phys_buffer[rmu->msg_tx_ring.tx_slot];

	/* Increment enqueue pointer */
	omr = in_be32(&rmu->msg_regs->omr);
	out_be32(&rmu->msg_regs->omr, omr | RIO_MSG_OMR_MUI);

	/* Go to next descriptor */
	if (++rmu->msg_tx_ring.tx_slot == rmu->msg_tx_ring.size)
		rmu->msg_tx_ring.tx_slot = 0;
    


out:
	return ret;
}

/**
 * fsl_open_outb_mbox - Initialize MPC85xx outbound mailbox
 * @mport: Master port implementing the outbound message unit
 * @dev_id: Device specific pointer to pass on event
 * @mbox: Mailbox to open
 * @entries: Number of entries in the outbound mailbox ring
 *
 * Initializes buffer ring, request the outbound message interrupt,
 * and enables the outbound message unit. Returns %0 on success and
 * %-EINVAL or %-ENOMEM on failure.
 */
int
fsl_open_outb_mbox(struct rio_mport *mport, void *dev_id, int mbox, int entries)
{
	int i, j, rc = 0;
	struct rio_priv *priv = mport->priv;
	struct fsl_rmu *rmu = GET_RMM_HANDLE(mport);

//    printk("===>[%s]\n", __func__);

	if ((entries < RIO_MIN_TX_RING_SIZE) ||
		(entries > RIO_MAX_TX_RING_SIZE) || (!is_power_of_2(entries))) {
		rc = -EINVAL;
		goto out;
	}

	/* Initialize shadow copy ring */
	rmu->msg_tx_ring.dev_id = dev_id;
	rmu->msg_tx_ring.size = entries;

	for (i = 0; i < rmu->msg_tx_ring.size; i++) {
		rmu->msg_tx_ring.virt_buffer[i] =
			dma_alloc_coherent(priv->dev, RIO_MSG_BUFFER_SIZE,
				&rmu->msg_tx_ring.phys_buffer[i], GFP_KERNEL);
		if (!rmu->msg_tx_ring.virt_buffer[i]) {
			rc = -ENOMEM;
			for (j = 0; j < rmu->msg_tx_ring.size; j++)
				if (rmu->msg_tx_ring.virt_buffer[j])
					dma_free_coherent(priv->dev,
							RIO_MSG_BUFFER_SIZE,
							rmu->msg_tx_ring.
							virt_buffer[j],
							rmu->msg_tx_ring.
							phys_buffer[j]);
			goto out;
		}
	}

	/* Initialize outbound message descriptor ring */
	rmu->msg_tx_ring.virt = dma_alloc_coherent(priv->dev,
				rmu->msg_tx_ring.size * RIO_MSG_DESC_SIZE,
				&rmu->msg_tx_ring.phys, GFP_KERNEL);
	if (!rmu->msg_tx_ring.virt) {
		rc = -ENOMEM;
		goto out_dma;
	}
	memset(rmu->msg_tx_ring.virt, 0,
			rmu->msg_tx_ring.size * RIO_MSG_DESC_SIZE);
	rmu->msg_tx_ring.tx_slot = 0;

	/* Point dequeue/enqueue pointers at first entry in ring */
	out_be32(&rmu->msg_regs->odqdpar, rmu->msg_tx_ring.phys);
	out_be32(&rmu->msg_regs->odqepar, rmu->msg_tx_ring.phys);

	/* Configure for snooping 允许处理程序缓存本地内存读取数据*/
	out_be32(&rmu->msg_regs->osar, 0x00000004);

	/* Clear interrupt status */
	out_be32(&rmu->msg_regs->osr, 0x000000b3);

	/* Hook up outbound message handler */
	rc = request_irq(IRQ_RIO_TX(mport), fsl_rio_tx_handler, 0,
			 "msg_tx", (void *)mport);
	if (rc < 0)
		goto out_irq;

	/*
	 * Configure outbound message unit
	 *      Snooping
	 *      Interrupts (all enabled, except QEIE)
	 *      Chaining mode
	 *      Disable
	 */
	out_be32(&rmu->msg_regs->omr, 0x00100220);

	/* Set number of entries */
	out_be32(&rmu->msg_regs->omr,
		 in_be32(&rmu->msg_regs->omr) |
		 ((get_bitmask_order(entries) - 2) << 12));

	/* Now enable the unit */
	out_be32(&rmu->msg_regs->omr, in_be32(&rmu->msg_regs->omr) | 0x1);

out:
	return rc;

out_irq:
	dma_free_coherent(priv->dev,
		rmu->msg_tx_ring.size * RIO_MSG_DESC_SIZE,
		rmu->msg_tx_ring.virt, rmu->msg_tx_ring.phys);

out_dma:
	for (i = 0; i < rmu->msg_tx_ring.size; i++)
		dma_free_coherent(priv->dev, RIO_MSG_BUFFER_SIZE,
		rmu->msg_tx_ring.virt_buffer[i],
		rmu->msg_tx_ring.phys_buffer[i]);

	return rc;
}

/**
 * fsl_close_outb_mbox - Shut down MPC85xx outbound mailbox
 * @mport: Master port implementing the outbound message unit
 * @mbox: Mailbox to close
 *
 * Disables the outbound message unit, free all buffers, and
 * frees the outbound message interrupt.
 */
void fsl_close_outb_mbox(struct rio_mport *mport, int mbox)
{
	struct rio_priv *priv = mport->priv;
	struct fsl_rmu *rmu = GET_RMM_HANDLE(mport);


    printk("===>[%s]\n", __func__);
	/* Disable inbound message unit */
	out_be32(&rmu->msg_regs->omr, 0);

	/* Free ring */
	dma_free_coherent(priv->dev,
	rmu->msg_tx_ring.size * RIO_MSG_DESC_SIZE,
	rmu->msg_tx_ring.virt, rmu->msg_tx_ring.phys);

	/* Free interrupt */
	free_irq(IRQ_RIO_TX(mport), (void *)mport);
}
#if 0
/*****************************************************************************
 func : fsl_open_inb_mbox
 description : RapidIO接收message 初始化
 input : 
 output :
 return :
 作者:聂飞
 时间:2013-11-26
*****************************************************************************/

int
fsl_open_inb_mbox(struct rio_mport *mport, void *dev_id, int mbox, int entries)

{
	int rc = 0;
	int uiVal = 0;
	void *pAddr;
	//struct rio_priv *priv = mport->priv;
	struct fsl_rmu *rmu = GET_RMM_HANDLE(mport);

	if ((entries < RIO_MIN_RX_RING_SIZE) ||
		(entries > RIO_MAX_RX_RING_SIZE) || (!is_power_of_2(entries))) {
		rc = -EINVAL;
		goto out;
	}
	pAddr = (void *)kmalloc(0x1000,GFP_KERNEL);
// DISABLE INTERUPUT
	uiVal = in_be32(&rmu->msg_regs->imr);
	uiVal &= (~(1<<0));
	out_be32(&rmu->msg_regs->imr, uiVal);
	/* Clear interrupt status */
	out_be32(&rmu->msg_regs->isr, 0x00000491);
	
	uiVal = in_be32(&rmu->msg_regs->imr);
	uiVal &= 0x0;
	out_be32(&rmu->msg_regs->imr, uiVal);

	uiVal = in_be32(&rmu->msg_regs->imr);
	uiVal |= ((1<<20) | (1 << 6) | (1 << 8) | (0xa << 12)|
		      (0xb << 16)|(0x1 << 28));
	out_be32(&rmu->msg_regs->imr, uiVal);
	

	out_be32(&rmu->msg_regs->ifqdpar, (u32)pAddr);
	out_be32(&rmu->msg_regs->ifqepar,(u32)pAddr);

	uiVal = in_be32(&rmu->msg_regs->imr);
	uiVal |= 0x01;
	out_be32(&rmu->msg_regs->imr, uiVal);

	/* Hook up inbound message handler */
	rc = request_irq(IRQ_RIO_RX(mport), fsl_rio_rx_handler, 0,
			 "msg_rx", (void *)mport);
	if (rc < 0) {
		goto out;
	}



out:
	return rc;
}

#else
/**
 * fsl_open_inb_mbox - Initialize MPC85xx inbound mailbox
 * @mport: Master port implementing the inbound message unit
 * @dev_id: Device specific pointer to pass on event
 * @mbox: Mailbox to open
 * @entries: Number of entries in the inbound mailbox ring
 *
 * Initializes buffer ring, request the inbound message interrupt,
 * and enables the inbound message unit. Returns %0 on success
 * and %-EINVAL or %-ENOMEM on failure.
 */
int
fsl_open_inb_mbox(struct rio_mport *mport, void *dev_id, int mbox, int entries)
{
	int i, rc = 0;
	struct rio_priv *priv = mport->priv;
	struct fsl_rmu *rmu = GET_RMM_HANDLE(mport);

	if ((entries < RIO_MIN_RX_RING_SIZE) ||
		(entries > RIO_MAX_RX_RING_SIZE) || (!is_power_of_2(entries))) {
		rc = -EINVAL;
		goto out;
	}

	/* Initialize client buffer ring */
	rmu->msg_rx_ring.dev_id = dev_id;
	rmu->msg_rx_ring.size = entries;
	rmu->msg_rx_ring.rx_slot = 0;
	for (i = 0; i < rmu->msg_rx_ring.size; i++)
		rmu->msg_rx_ring.virt_buffer[i] = NULL;

	/* Initialize inbound message ring */
	rmu->msg_rx_ring.virt = dma_alloc_coherent(priv->dev,
				rmu->msg_rx_ring.size * RIO_MAX_MSG_SIZE,
				&rmu->msg_rx_ring.phys, GFP_KERNEL);
	if (!rmu->msg_rx_ring.virt) {
		rc = -ENOMEM;
		goto out;
	}

	/* Point dequeue/enqueue pointers at first entry in ring */
	out_be32(&rmu->msg_regs->ifqdpar, (u32) rmu->msg_rx_ring.phys);
	out_be32(&rmu->msg_regs->ifqepar, (u32) rmu->msg_rx_ring.phys);

	/* Clear interrupt status */
	out_be32(&rmu->msg_regs->isr, 0x00000091);

	/* Hook up inbound message handler */
	rc = request_irq(IRQ_RIO_RX(mport), fsl_rio_rx_handler, 0,
			 "msg_rx", (void *)mport);
	if (rc < 0) {
		dma_free_coherent(priv->dev, RIO_MSG_BUFFER_SIZE,
			rmu->msg_tx_ring.virt_buffer[i],
			rmu->msg_tx_ring.phys_buffer[i]);
		goto out;
	}

	/*
	 * Configure inbound message unit:
	 *      Snooping
	 *      4KB max message size
	 *      Unmask all interrupt sources
	 *      Disable
	 */
	out_be32(&rmu->msg_regs->imr, 0x001b0060);

	/* Set number of queue entries */
	setbits32(&rmu->msg_regs->imr, (get_bitmask_order(entries) - 2) << 12);

	/* Now enable the unit */
	setbits32(&rmu->msg_regs->imr, 0x1);

out:
	return rc;
}
#endif
/**
 * fsl_close_inb_mbox - Shut down MPC85xx inbound mailbox
 * @mport: Master port implementing the inbound message unit
 * @mbox: Mailbox to close
 *
 * Disables the inbound message unit, free all buffers, and
 * frees the inbound message interrupt.
 */
void fsl_close_inb_mbox(struct rio_mport *mport, int mbox)
{
	struct rio_priv *priv = mport->priv;
	struct fsl_rmu *rmu = GET_RMM_HANDLE(mport);

	/* Disable inbound message unit */
	out_be32(&rmu->msg_regs->imr, 0);

	/* Free ring */
	dma_free_coherent(priv->dev, rmu->msg_rx_ring.size * RIO_MAX_MSG_SIZE,
	rmu->msg_rx_ring.virt, rmu->msg_rx_ring.phys);

	/* Free interrupt */
	free_irq(IRQ_RIO_RX(mport), (void *)mport);
}

/**
 * fsl_add_inb_buffer - Add buffer to the MPC85xx inbound message queue
 * @mport: Master port implementing the inbound message unit
 * @mbox: Inbound mailbox number
 * @buf: Buffer to add to inbound queue
 *
 * Adds the @buf buffer to the MPC85xx inbound message queue. Returns
 * %0 on success or %-EINVAL on failure.
 */
int fsl_add_inb_buffer(struct rio_mport *mport, int mbox, void *buf)
{
	int rc = 0;
	struct fsl_rmu *rmu = GET_RMM_HANDLE(mport);

	pr_debug("RIO: fsl_add_inb_buffer(), msg_rx_ring.rx_slot %d\n",
		 rmu->msg_rx_ring.rx_slot);

	if (rmu->msg_rx_ring.virt_buffer[rmu->msg_rx_ring.rx_slot]) {
		printk(KERN_ERR
			"RIO: error adding inbound buffer %d, buffer exists\n",
			rmu->msg_rx_ring.rx_slot);
		rc = -EINVAL;
		goto out;
	}
/*rmu->msg_rx_ring.size在open函数中完成初始化，按照正常流程设计add_inb_buffer
放在fsl_open_inb_mbox函数之后，可以使用一块连续物理空间完成所有
rmu->msg_rx_ring.virt_buffer[0-entry]初始化，之后不再申请内存和add_inb_buffer
也可以设计成首先判空rmu->msg_rx_ring.virt_buffer[rmu->msg_rx_ring.rx_slot]，根据需要申请
内存空间*/
	rmu->msg_rx_ring.virt_buffer[rmu->msg_rx_ring.rx_slot] = buf;
	if (++rmu->msg_rx_ring.rx_slot == rmu->msg_rx_ring.size)
		rmu->msg_rx_ring.rx_slot = 0;

out:
	return rc;
}

/**
 * fsl_get_inb_message - Fetch inbound message from the MPC85xx message unit
 * @mport: Master port implementing the inbound message unit
 * @mbox: Inbound mailbox number
 *
 * Gets the next available inbound message from the inbound message queue.
 * A pointer to the message is returned on success or NULL on failure.
 */
void *fsl_get_inb_message(struct rio_mport *mport, int mbox)
{
	struct fsl_rmu *rmu = GET_RMM_HANDLE(mport);
	u32 phys_buf;
	void *virt_buf;
	void *buf = NULL;
	int buf_idx;

	phys_buf = in_be32(&rmu->msg_regs->ifqdpar);

	/* If no more messages, then bail out */
	if (phys_buf == in_be32(&rmu->msg_regs->ifqepar))
		goto out2;

	virt_buf = rmu->msg_rx_ring.virt + (phys_buf
						- rmu->msg_rx_ring.phys);
	buf_idx = (phys_buf - rmu->msg_rx_ring.phys) / RIO_MAX_MSG_SIZE;
	buf = rmu->msg_rx_ring.virt_buffer[buf_idx];

	if (!buf) {
		printk(KERN_ERR
			"RIO: inbound message copy failed, no buffers\n");
		goto out1;
	}

	/* Copy max message size, caller is expected to allocate that big */
	memcpy(buf, virt_buf, RIO_MAX_MSG_SIZE);

	/* Clear the available buffer */
	rmu->msg_rx_ring.virt_buffer[buf_idx] = NULL;

out1:
	setbits32(&rmu->msg_regs->imr, RIO_MSG_IMR_MI);

out2:
	return buf;
}

/**
 * fsl_rio_doorbell_init - MPC85xx doorbell interface init
 * @mport: Master port implementing the inbound doorbell unit
 *
 * Initializes doorbell unit hardware and inbound DMA buffer
 * ring. Called from fsl_rio_setup(). Returns %0 on success
 * or %-ENOMEM on failure.
 */
int fsl_rio_doorbell_init(struct fsl_rio_dbell *dbell)
{
	int rc = 0;

	/* Initialize inbound doorbells */
	dbell->dbell_ring.virt = dma_alloc_coherent(dbell->dev, 512 *
		DOORBELL_MESSAGE_SIZE, &dbell->dbell_ring.phys, GFP_KERNEL);
	if (!dbell->dbell_ring.virt) {
		printk(KERN_ERR "RIO: unable allocate inbound doorbell ring\n");
		rc = -ENOMEM;
		goto out;
	}

	/* Point dequeue/enqueue pointers at first entry in ring */
	out_be32(&dbell->dbell_regs->dqdpar, (u32) dbell->dbell_ring.phys);
	out_be32(&dbell->dbell_regs->dqepar, (u32) dbell->dbell_ring.phys);

	/* Clear interrupt status */
	out_be32(&dbell->dbell_regs->dsr, 0x00000091);

	/* Hook up doorbell handler */
	rc = request_irq(IRQ_RIO_BELL(dbell), fsl_rio_dbell_handler, 0,
			 "dbell_rx", (void *)dbell);
	if (rc < 0) {
		dma_free_coherent(dbell->dev, 512 * DOORBELL_MESSAGE_SIZE,
			 dbell->dbell_ring.virt, dbell->dbell_ring.phys);
		printk(KERN_ERR
			"MPC85xx RIO: unable to request inbound doorbell irq");
		goto out;
	}

	/* Configure doorbells for snooping, 512 entries, and enable */
	out_be32(&dbell->dbell_regs->dmr, 0x00108161);

out:
	return rc;
}

int fsl_rio_setup_rmu(struct rio_mport *mport, struct device_node *node)
{
	struct rio_priv *priv;
	struct fsl_rmu *rmu;
	u64 msg_start;
	const u32 *msg_addr;
	int mlen;
	int aw;

	if (!mport || !mport->priv)
		return -EINVAL;

	priv = mport->priv;

	if (!node) {
		dev_warn(priv->dev, "Can't get %s property 'fsl,rmu'\n",
			priv->dev->of_node->full_name);
		return -EINVAL;
	}

	rmu = kzalloc(sizeof(struct fsl_rmu), GFP_KERNEL);
	if (!rmu)
		return -ENOMEM;

	aw = of_n_addr_cells(node);
	msg_addr = of_get_property(node, "reg", &mlen);
	if (!msg_addr) {
		pr_err("%s: unable to find 'reg' property of message-unit\n",
			node->full_name);
		kfree(rmu);
		return -ENOMEM;
	}
	msg_start = of_read_number(msg_addr, aw);

	rmu->msg_regs = (struct rio_msg_regs *)
			(rmu_regs_win + (u32)msg_start);

	rmu->txirq = irq_of_parse_and_map(node, 0);
	rmu->rxirq = irq_of_parse_and_map(node, 1);
	printk(KERN_INFO "%s: txirq: %d, rxirq %d\n",
		node->full_name, rmu->txirq, rmu->rxirq);

	priv->rmm_handle = rmu;

	rio_init_dbell_res(&mport->riores[RIO_DOORBELL_RESOURCE], 0, 0xffff);
	rio_init_mbox_res(&mport->riores[RIO_INB_MBOX_RESOURCE], 0, 0);
	rio_init_mbox_res(&mport->riores[RIO_OUTB_MBOX_RESOURCE], 0, 0);

	return 0;
}
