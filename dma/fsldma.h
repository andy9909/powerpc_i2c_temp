/*
 * Copyright (C) 2007-2010 Freescale Semiconductor, Inc. All rights reserved.
 *
 * Author:
 *   Zhang Wei <wei.zhang@freescale.com>, Jul 2007
 *   Ebony Zhu <ebony.zhu@freescale.com>, May 2007
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */
#ifndef __DMA_FSLDMA_H
#define __DMA_FSLDMA_H

#include <linux/device.h>
#include <linux/dmapool.h>
#include <linux/dmaengine.h>

/* Define data structures needed by Freescale
 * MPC8540 and MPC8349 DMA controller.
 */
#define FSL_DMA_MR_CS		0x00000001
#define FSL_DMA_MR_CC		0x00000002
#define FSL_DMA_MR_CA		0x00000008
#define FSL_DMA_MR_EIE		0x00000040
#define FSL_DMA_MR_XFE		0x00000020
#define FSL_DMA_MR_EOLNIE	0x00000100
#define FSL_DMA_MR_EOLSIE	0x00000080
#define FSL_DMA_MR_EOSIE	0x00000200
#define FSL_DMA_MR_CDSM		0x00000010
#define FSL_DMA_MR_CTM		0x00000004
#define FSL_DMA_MR_EMP_EN	0x00200000
#define FSL_DMA_MR_EMS_EN	0x00040000
#define FSL_DMA_MR_DAHE		0x00002000
#define FSL_DMA_MR_SAHE		0x00001000

/*
 * Bandwidth/pause control determines how many bytes a given
 * channel is allowed to transfer before the DMA engine pauses
 * the current channel and switches to the next channel
 */
#define FSL_DMA_MR_BWC         0x08000000

/* Special MR definition for MPC8349 */
#define FSL_DMA_MR_EOTIE	0x00000080
#define FSL_DMA_MR_PRC_RM	0x00000800

#define FSL_DMA_SR_CH		0x00000020
#define FSL_DMA_SR_PE		0x00000010
#define FSL_DMA_SR_CB		0x00000004
#define FSL_DMA_SR_TE		0x00000080
#define FSL_DMA_SR_EOSI		0x00000002
#define FSL_DMA_SR_EOLSI	0x00000001
#define FSL_DMA_SR_EOCDI	0x00000001
#define FSL_DMA_SR_EOLNI	0x00000008

#define FSL_DMA_SATR_SBPATMU			0x20000000
#define FSL_DMA_SATR_STRANSINT_RIO		0x00c00000
#define FSL_DMA_SATR_SREADTYPE_SNOOP_READ	0x00050000
#define FSL_DMA_SATR_SREADTYPE_BP_IORH		0x00020000
#define FSL_DMA_SATR_SREADTYPE_BP_NREAD		0x00040000
#define FSL_DMA_SATR_SREADTYPE_BP_MREAD		0x00070000

#define FSL_DMA_DATR_DBPATMU			0x20000000
#define FSL_DMA_DATR_DTRANSINT_RIO		0x00c00000
#define FSL_DMA_DATR_DWRITETYPE_SNOOP_WRITE	0x00050000
#define FSL_DMA_DATR_DWRITETYPE_BP_FLUSH	0x00010000

#define FSL_DMA_EOL		((u64)0x1)
#define FSL_DMA_SNEN		((u64)0x10)
#define FSL_DMA_EOSIE		0x8
#define FSL_DMA_NLDA_MASK	(~(u64)0x1f)

#define FSL_DMA_BCR_MAX_CNT	0x03ffffffu

#define FSL_DMA_DGSR_TE		0x80
#define FSL_DMA_DGSR_CH		0x20
#define FSL_DMA_DGSR_PE		0x10
#define FSL_DMA_DGSR_EOLNI	0x08
#define FSL_DMA_DGSR_CB		0x04
#define FSL_DMA_DGSR_EOSI	0x02
#define FSL_DMA_DGSR_EOLSI	0x01

/*housir: added by housir  */
#define DMA8641_MAX_CHAN                4
#define DMA8641_SATR_SBPATMU_MASK       (0x1<<29)
#define DMA8641_SATR_SBPATMU_NORMAL     (0x0<<29)
#define DMA8641_SATR_SBPATMU_BYPASS     (0x1<<29)


/* Source Transaction Interface (SRIO only, applicable while SPBATMU is set)*/
#define DMA8641_SATR_STRANSIT_MASK      ((0xF<<20) | DMA8641_SATR_SBPATMU_MASK)
#define DMA8641_SATR_STRANSIT_PCIE1     ((0x0<<20) | DMA8641_SATR_SBPATMU_BYPASS)
#define DMA8641_SATR_STRANSIT_PCIE2     ((0x1<<20) | DMA8641_SATR_SBPATMU_BYPASS)
#define DMA8641_SATR_STRANSIT_SRIO      ((0xC<<20) | DMA8641_SATR_SBPATMU_BYPASS)

/* DMA Source transaction type */
#define DMA8641_SATR_SREADTTYPE_MASK        ((0xF<<16) | DMA8641_SATR_SBPATMU_MASK)
#define DMA8641_SATR_SREADTTYPE_ATMU_IORD   ((0x2<<16) | DMA8641_SATR_SBPATMU_BYPASS)
#define DMA8641_SATR_SREADTTYPE_ATMU_NRD    ((0x4<<16) | DMA8641_SATR_SBPATMU_BYPASS)
#define DMA8641_SATR_SREADTTYPE_ATMU_MRD    ((0x7<<16) | DMA8641_SATR_SBPATMU_BYPASS)
#define DMA8641_SATR_SREADTTYPE_RD_NOSNOOP  ((0x4<<16) | DMA8641_SATR_SBPATMU_NORMAL)
#define DMA8641_SATR_SREADTTYPE_RD_SNOOP    ((0x5<<16) | DMA8641_SATR_SBPATMU_NORMAL)

#define DMA8641_DATR_DBPATMU_MASK       (0x1<<29)
#define DMA8641_DATR_DBPATMU_NORMAL     (0x0<<29)
#define DMA8641_DATR_DBPATMU_BYPASS     (0x1<<29)


#define DMA8641_DATR_DWRITETTYPE_MASK           ((0xF<<16) | DMA8641_DATR_DBPATMU_MASK)
#define DMA8641_DATR_DWRITETTYPE_ATMU_FLUSH     ((0x1<<16) | DMA8641_DATR_DBPATMU_BYPASS)
#define DMA8641_DATR_DWRITETTYPE_ATMU_SWRITE    ((0x3<<16) | DMA8641_DATR_DBPATMU_BYPASS)
#define DMA8641_DATR_DWRITETTYPE_ATMU_NWRITE    ((0x4<<16) | DMA8641_DATR_DBPATMU_BYPASS)
#define DMA8641_DATR_DWRITETTYPE_ATMU_NWRITE_R  ((0x5<<16) | DMA8641_DATR_DBPATMU_BYPASS)
#define DMA8641_DATR_DWRITETTYPE_ATMU_MSG_SMALL ((0x6<<16) | DMA8641_DATR_DBPATMU_BYPASS)
#define DMA8641_DATR_DWRITETTYPE_ATMU_MWRITE    ((0x7<<16) | DMA8641_DATR_DBPATMU_BYPASS)
#define DMA8641_DATR_DWRITETTYPE_WR_NOSNOOP     ((0x4<<16) | DMA8641_DATR_DBPATMU_NORMAL)
#define DMA8641_DATR_DWRITETTYPE_WR_SNOOP       ((0x5<<16) | DMA8641_DATR_DBPATMU_NORMAL)


#define LOCAL_SRC_ATTRIB_NOSNOOP      (DMA8641_SATR_SREADTTYPE_RD_NOSNOOP)
#define LOCAL_SRC_ATTRIB_SNOOP      (DMA8641_SATR_SREADTTYPE_RD_SNOOP)

#define LOCAL_DST_ATTRIB_NOSNOOP       (DMA8641_DATR_DWRITETTYPE_WR_NOSNOOP)
#define LOCAL_DST_ATTRIB_SNOOP       (DMA8641_DATR_DWRITETTYPE_WR_SNOOP)

/* RapidIO transaction flow level (SRIO only) */
#define DMA8641_SATR_STFLOWLVL_MASK     ((0x3<<26) | DMA8641_SATR_SBPATMU_MASK)
#define DMA8641_SATR_STFLOWLVL_LOW      ((0x0<<26) | DMA8641_SATR_SBPATMU_BYPASS)
#define DMA8641_SATR_STFLOWLVL_MED      ((0x1<<26) | DMA8641_SATR_SBPATMU_BYPASS)
#define DMA8641_SATR_STFLOWLVL_HIGH     ((0x2<<26) | DMA8641_SATR_SBPATMU_BYPASS)
/* RapidIO transaction flow level (SRIO only) */
#define DMA8641_DATR_DTFLOWLVL_MASK     ((0x3<<26) | DMA8641_DATR_DBPATMU_MASK)
#define DMA8641_DATR_DTFLOWLVL_LOW      ((0x0<<26) | DMA8641_DATR_DBPATMU_BYPASS)
#define DMA8641_DATR_DTFLOWLVL_MED      ((0x1<<26) | DMA8641_DATR_DBPATMU_BYPASS)
#define DMA8641_DATR_DTFLOWLVL_HIGH     ((0x2<<26) | DMA8641_DATR_DBPATMU_BYPASS)
/* Destination Transaction Interface (SRIO only, applicable while SPBATMU is set)*/
#define DMA8641_DATR_DTRANSIT_MASK      ((0xF<<20) | DMA8641_DATR_DBPATMU_MASK)
#define DMA8641_DATR_DTRANSIT_PCIE1     ((0x0<<20) | DMA8641_DATR_DBPATMU_BYPASS)
#define DMA8641_DATR_DTRANSIT_PCIE2     ((0x1<<20) | DMA8641_DATR_DBPATMU_BYPASS)
#define DMA8641_DATR_DTRANSIT_SRIO      ((0xC<<20) | DMA8641_DATR_DBPATMU_BYPASS)

/* SRIO specific target ID */
#define DMA8641_SATR_SRIO_ID_MASK       ((0xFF<<2) | DMA8641_SATR_SBPATMU_MASK)
#define DMA8641_SATR_SRIO_ID(id)        (((id<<2) & (0xFF<<2)) | DMA8641_SATR_SBPATMU_BYPASS)

/* SRIO specific target ID */
#define DMA8641_DATR_SRIO_ID_MASK       ((0xFF<<2) | DMA8641_DATR_DBPATMU_MASK)
#define DMA8641_DATR_SRIO_ID(id)        (((id<<2) & (0xFF<<2)) | DMA8641_DATR_DBPATMU_BYPASS)


#define RAPIDIO_SRC_ATTRIB      (DMA8641_SATR_SBPATMU_BYPASS | \
                                 DMA8641_SATR_STFLOWLVL_HIGH | \
                                 DMA8641_SATR_STRANSIT_SRIO | \
                                 DMA8641_SATR_SREADTTYPE_ATMU_NRD)
#define RAPIDIO_DST_ATTRIB      (DMA8641_DATR_DBPATMU_BYPASS | \
                                 DMA8641_DATR_DTFLOWLVL_HIGH | \
                                 DMA8641_DATR_DTRANSIT_SRIO |\
                                 DMA8641_DATR_DWRITETTYPE_ATMU_NWRITE)




#define MPC8641_DMA_BWC_DISABLE      (0xF<<24)

#define MPC8641_DMA_DAHTS_DISABLE    (0x0<<13)
#define MPC8641_DMA_DAHTS_1BYTE      ((0x0<<16) | (0x1<<13))
#define MPC8641_DMA_DAHTS_2BYTES     ((0x1<<16) | (0x1<<13))
#define MPC8641_DMA_DAHTS_4BYTES     ((0x2<<16) | (0x1<<13))
#define MPC8641_DMA_DAHTS_8BYTES     ((0x3<<16) | (0x1<<13))

/* Source address hold transfer size */
#define MPC8641_DMA_SAHTS_DISABLE    (0x0<<12)
#define MPC8641_DMA_SAHTS_1BYTE      ((0x0<<14) | (0x1<<12))
#define MPC8641_DMA_SAHTS_2BYTES     ((0x1<<14) | (0x1<<12))
#define MPC8641_DMA_SAHTS_4BYTES     ((0x2<<14) | (0x1<<12))
#define MPC8641_DMA_SAHTS_8BYTES     ((0x3<<14) | (0x1<<12))


#define MPC8641_DMA_DEF_OPTIONS      (MPC8641_DMA_BWC_DISABLE    | \
                                      MPC8641_DMA_DAHTS_DISABLE  | \
                                      MPC8641_DMA_SAHTS_DISABLE)
                                      

#define DMA_WAIT_TIMEOUT  sysClkRateGet() /*tick*/
#define DMA_MODULE_OFFSET 0x21000
#define DMA8641_MR_CTM_MASK         (0x1<<2)
#define DMA8641_MR_CTM_CHAINING     (0x0<<2)
#define DMA8641_MR_CTM_DIRECT       (0x1<<2)

/* Channel Continue */
#define DMA8641_MR_CC_MASK          (0x1<<1)
#define DMA8641_MR_CC_NO            (0x0<<1)
#define DMA8641_MR_CC_YES           (0x1<<1)

#define DMA8641_MR_CS		0x00000001
#define DMA8641_MR_CC		0x00000002
#define DMA8641_MR_CA		0x00000008
#define DMA8641_MR_EMP_EN	0x00200000


/* Channel halt/start */
#define DMA8641_MR_CS_MASK          (0x1<<0)
#define DMA8641_MR_CS_HALT          (0x0<<0)
#define DMA8641_MR_CS_START         (0x1<<0)

/* Channel busy status  */
#define DMA8641_SR_CB_MASK          (0x1<<2)
#define DMA8641_SR_CB_IDLE          (0x0<<2)
#define DMA8641_SR_CB_BUSY          (0x1<<2)

/* End-of-Segment interrupt (Write 1 to reset bit) */
#define DMA8641_SR_EOSI_MASK        (0x1<<1)
#define DMA8641_SR_EOSI_NO          (0x0<<1)
#define DMA8641_SR_EOSI_ACTIVE      (0x1<<1)

/* End-of-List interrupt (Write 1 to reset bit) */
#define DMA8641_SR_EOLSI_MASK        (0x1<<0)
#define DMA8641_SR_EOLSI_NO          (0x0<<0)
#define DMA8641_SR_EOLSI_ACTIVE      (0x1<<0)

#define DMA8641_SR_TE_MASK          (0x1<<7)
#define DMA8641_SR_TE_NOERROR       (0x0<<7)
#define DMA8641_SR_TE_ERROR         (0x1<<7)


#define  MPC8641_DMA_MR              0x00
#define  MPC8641_DMA_SR              0x04
#define  MPC8641_DMA_SATR            0x10
#define  MPC8641_DMA_SAR             0x14
#define  MPC8641_DMA_DATR            0x18
#define  MPC8641_DMA_DAR             0x1C
#define  MPC8641_DMA_BCR             0x20


typedef u64 __bitwise v64;
typedef u32 __bitwise v32;

struct fsl_dma_ld_hw {
	v64 src_addr;/*包含源地址和源地址属性寄存器*/
	v64 dst_addr;/*u64包含目的地址和目的地址属性寄存器*/
	v64 next_ln_addr;
	v32 count;
	v32 reserve;
} __attribute__((aligned(32)));

struct fsl_desc_sw {
	struct fsl_dma_ld_hw hw;
	struct list_head node;
	struct list_head tx_list;
	struct dma_async_tx_descriptor async_tx;
} __attribute__((aligned(32)));

struct fsldma_chan_regs {
	u32 mr;		/* 0x00 - Mode Register */
	u32 sr;		/* 0x04 - Status Register */
	u64 cdar;	/* 0x08 - Current descriptor address register */
	u64 sar;	/* 0x10 - Source Address Register U64 包含了源地址属性寄存器*/
	u64 dar;	/* 0x18 - Destination Address Register  U64 包含了目的地址属性寄存器*/
	u32 bcr;	/* 0x20 - Byte Count Register */
	u64 ndar;	/* 0x24 - Next Descriptor Address Register */
};

struct fsldma_chan;
#define FSL_DMA_MAX_CHANS_PER_DEVICE 4

struct fsldma_device {
	void __iomem *regs;	/* DGSR register base */
	struct device *dev;
	struct dma_device common;
	struct fsldma_chan *chan[FSL_DMA_MAX_CHANS_PER_DEVICE];
	u32 feature;		/* The same as DMA channels */
	int irq;		/* Channel IRQ */
};

/* Define macros for fsldma_chan->feature property */
#define FSL_DMA_LITTLE_ENDIAN	0x00000000
#define FSL_DMA_BIG_ENDIAN	0x00000001

#define FSL_DMA_IP_MASK		0x00000ff0
#define FSL_DMA_IP_85XX		0x00000010
#define FSL_DMA_IP_83XX		0x00000020

#define FSL_DMA_CHAN_PAUSE_EXT	0x00001000
#define FSL_DMA_CHAN_START_EXT	0x00002000

struct fsldma_chan {
	char name[8];			/* Channel name */
	struct fsldma_chan_regs __iomem *regs;
	spinlock_t desc_lock;		/* Descriptor operation lock */
	struct list_head ld_pending;	/* Link descriptors queue */
	struct list_head ld_running;	/* Link descriptors queue */
	struct dma_chan common;		/* DMA common channel */
	struct dma_pool *desc_pool;	/* Descriptors pool */
	struct device *dev;		/* Channel device */
	int irq;			/* Channel IRQ */
	int id;				/* Raw id of this channel */
	struct tasklet_struct tasklet;
	u32 feature;
	bool idle;			/* DMA controller is idle */

	void (*toggle_ext_pause)(struct fsldma_chan *fsl_chan, int enable);
	void (*toggle_ext_start)(struct fsldma_chan *fsl_chan, int enable);
	void (*set_src_loop_size)(struct fsldma_chan *fsl_chan, int size);
	void (*set_dst_loop_size)(struct fsldma_chan *fsl_chan, int size);
	void (*set_request_count)(struct fsldma_chan *fsl_chan, int size);
};

#define to_fsl_chan(chan) container_of(chan, struct fsldma_chan, common)
#define to_fsl_desc(lh) container_of(lh, struct fsl_desc_sw, node)
#define tx_to_fsl_desc(tx) container_of(tx, struct fsl_desc_sw, async_tx)

#ifndef __powerpc64__
static u64 in_be64(const u64 __iomem *addr)
{
	return ((u64)in_be32((u32 __iomem *)addr) << 32) |
		(in_be32((u32 __iomem *)addr + 1));
}

static void out_be64(u64 __iomem *addr, u64 val)
{
	out_be32((u32 __iomem *)addr, val >> 32);
	out_be32((u32 __iomem *)addr + 1, (u32)val);
}

/* There is no asm instructions for 64 bits reverse loads and stores */
static u64 in_le64(const u64 __iomem *addr)
{
	return ((u64)in_le32((u32 __iomem *)addr + 1) << 32) |
		(in_le32((u32 __iomem *)addr));
}

static void out_le64(u64 __iomem *addr, u64 val)
{
	out_le32((u32 __iomem *)addr + 1, val >> 32);
	out_le32((u32 __iomem *)addr, (u32)val);
}
#endif

#define DMA_IN(fsl_chan, addr, width)					\
		(((fsl_chan)->feature & FSL_DMA_BIG_ENDIAN) ?		\
			in_be##width(addr) : in_le##width(addr))
#define DMA_OUT(fsl_chan, addr, val, width)				\
		(((fsl_chan)->feature & FSL_DMA_BIG_ENDIAN) ?		\
			out_be##width(addr, val) : out_le##width(addr, val))

#define DMA_TO_CPU(fsl_chan, d, width)					\
		(((fsl_chan)->feature & FSL_DMA_BIG_ENDIAN) ?		\
			be##width##_to_cpu((__force __be##width)(v##width)d) : \
			le##width##_to_cpu((__force __le##width)(v##width)d))
#define CPU_TO_DMA(fsl_chan, c, width)					\
		(((fsl_chan)->feature & FSL_DMA_BIG_ENDIAN) ?		\
			(__force v##width)cpu_to_be##width(c) :		\
			(__force v##width)cpu_to_le##width(c))

#endif	/* __DMA_FSLDMA_H */
