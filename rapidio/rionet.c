/*
 * rionet - Ethernet driver over RapidIO messaging services
 *
 * Copyright 2005 MontaVista Software, Inc.
 * Matt Porter <mporter@kernel.crashing.org>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/rio.h>
#include <linux/rio_drv.h>
#include <linux/slab.h>
#include <linux/rio_ids.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/crc32.h>
#include <linux/ethtool.h>

#define DRV_NAME        "rionet"
#define DRV_VERSION     "0.2"
#define DRV_AUTHOR      "Matt Porter <mporter@kernel.crashing.org>"
#define DRV_DESC        "Ethernet over RapidIO"

MODULE_AUTHOR(DRV_AUTHOR);
MODULE_DESCRIPTION(DRV_DESC);
MODULE_LICENSE("GPL");

#define RIONET_DEFAULT_MSGLEVEL \
			(NETIF_MSG_DRV          | \
			 NETIF_MSG_LINK         | \
			 NETIF_MSG_RX_ERR       | \
			 NETIF_MSG_TX_ERR)

#define RIONET_DOORBELL_JOIN	0x1000
#define RIONET_DOORBELL_LEAVE	0x1001

#define RIONET_MAILBOX		0

#define RIONET_TX_RING_SIZE	CONFIG_RIONET_TX_SIZE
#define RIONET_RX_RING_SIZE	CONFIG_RIONET_RX_SIZE

static LIST_HEAD(rionet_peers);

struct rionet_private {
	struct rio_mport *mport;
	struct sk_buff *rx_skb[RIONET_RX_RING_SIZE];
	struct sk_buff *tx_skb[RIONET_TX_RING_SIZE];
	int rx_slot;
	int tx_slot;
	int tx_cnt;
	int ack_slot;
	spinlock_t lock;
	spinlock_t tx_lock;
	u32 msg_enable;
};

struct rionet_peer {
	struct list_head node;
	struct rio_dev *rdev;
	struct resource *res;
};

static int rionet_check = 0;
static int rionet_capable = 1;

/*
 * This is a fast lookup table for translating TX
 * Ethernet packets into a destination RIO device. It
 * could be made into a hash table to save memory depending
 * on system trade-offs.
 */
static struct rio_dev **rionet_active;
static int nact;	/* total number of active rionet peers */

/*housir: 用以获取自己的设备id */
extern struct rio_mport *mem_mport;



extern void test_nread(unsigned char * str);
extern void test_nwrite(unsigned char * str);


#define is_rionet_capable(src_ops, dst_ops)			\
			((src_ops & RIO_SRC_OPS_DATA_MSG) &&	\
			 (dst_ops & RIO_DST_OPS_DATA_MSG) &&	\
			 (src_ops & RIO_SRC_OPS_DOORBELL) &&	\
			 (dst_ops & RIO_DST_OPS_DOORBELL))
#define dev_rionet_capable(dev) \
	is_rionet_capable(dev->src_ops, dev->dst_ops)

#define RIONET_MAC_MATCH(x)	(!memcmp((x), "\00\01\00\01", 4))
#define RIONET_GET_DESTID(x)	((*((u8 *)x + 4) << 8) | *((u8 *)x + 5))

static int rionet_rx_clean(struct net_device *ndev)
{
	int i;
	int error = 0;
	struct rionet_private *rnet = netdev_priv(ndev);
	void *data;

	i = rnet->rx_slot;

	do {
		if (!rnet->rx_skb[i])
			continue;

		if (!(data = rio_get_inb_message(rnet->mport, RIONET_MAILBOX)))
			break;

		rnet->rx_skb[i]->data = data;
		skb_put(rnet->rx_skb[i], RIO_MAX_MSG_SIZE);
		rnet->rx_skb[i]->protocol =
		    eth_type_trans(rnet->rx_skb[i], ndev);
		error = netif_rx(rnet->rx_skb[i]);

		if (error == NET_RX_DROP) {
			ndev->stats.rx_dropped++;
		} else {
			ndev->stats.rx_packets++;
			ndev->stats.rx_bytes += RIO_MAX_MSG_SIZE;
		}

	} while ((i = (i + 1) % RIONET_RX_RING_SIZE) != rnet->rx_slot);

	return i;
}

static void rionet_rx_fill(struct net_device *ndev, int end)
{
	int i;
	struct rionet_private *rnet = netdev_priv(ndev);

	i = rnet->rx_slot;
	do {
		rnet->rx_skb[i] = dev_alloc_skb(RIO_MAX_MSG_SIZE);

		if (!rnet->rx_skb[i])
			break;

		rio_add_inb_buffer(rnet->mport, RIONET_MAILBOX,
				   rnet->rx_skb[i]->data);
	} while ((i = (i + 1) % RIONET_RX_RING_SIZE) != end);

	rnet->rx_slot = i;
}

static int rionet_queue_tx_msg(struct sk_buff *skb, struct net_device *ndev,
			       struct rio_dev *rdev)
{
	struct rionet_private *rnet = netdev_priv(ndev);

	rio_add_outb_message(rnet->mport, rdev, 0, skb->data, skb->len);
	rnet->tx_skb[rnet->tx_slot] = skb;

	ndev->stats.tx_packets++;
	ndev->stats.tx_bytes += skb->len;

	if (++rnet->tx_cnt == RIONET_TX_RING_SIZE)
		netif_stop_queue(ndev);

	++rnet->tx_slot;
	rnet->tx_slot &= (RIONET_TX_RING_SIZE - 1);

	if (netif_msg_tx_queued(rnet))
		printk(KERN_INFO "%s: queued skb len %8.8x\n", DRV_NAME,
		       skb->len);

	return 0;
}

static int rionet_start_xmit(struct sk_buff *skb, struct net_device *ndev)
{
	int i;
	struct rionet_private *rnet = netdev_priv(ndev);
	struct ethhdr *eth = (struct ethhdr *)skb->data;
	u16 destid;
	unsigned long flags;
	int add_num = 1;

	local_irq_save(flags);
	if (!spin_trylock(&rnet->tx_lock)) {
		local_irq_restore(flags);
		return NETDEV_TX_LOCKED;
	}

	if (is_multicast_ether_addr(eth->h_dest))
		add_num = nact;

	if ((rnet->tx_cnt + add_num) > RIONET_TX_RING_SIZE) {
		netif_stop_queue(ndev);
		spin_unlock_irqrestore(&rnet->tx_lock, flags);
		printk(KERN_ERR "%s: BUG! Tx Ring full when queue awake!\n",
		       ndev->name);
		return NETDEV_TX_BUSY;
	}

	if (is_multicast_ether_addr(eth->h_dest)) {
		int count = 0;
		for (i = 0; i < RIO_MAX_ROUTE_ENTRIES(rnet->mport->sys_size);
				i++)
			if (rionet_active[i]) {
				rionet_queue_tx_msg(skb, ndev,
						    rionet_active[i]);
				if (count)
					atomic_inc(&skb->users);
				count++;
			}
	} else if (RIONET_MAC_MATCH(eth->h_dest)) {
		destid = RIONET_GET_DESTID(eth->h_dest);
		if (rionet_active[destid])
			rionet_queue_tx_msg(skb, ndev, rionet_active[destid]);
	}

	spin_unlock_irqrestore(&rnet->tx_lock, flags);

	return NETDEV_TX_OK;
}

static void rionet_dbell_event(struct rio_mport *mport, void *dev_id, u16 sid, u16 tid,
			       u16 info)
{
	struct net_device *ndev = dev_id;
	struct rionet_private *rnet = netdev_priv(ndev);
	struct rionet_peer *peer;

	if (netif_msg_intr(rnet))
		printk(KERN_INFO "%s: doorbell sid %4.4x tid %4.4x info %4.4x",
		       DRV_NAME, sid, tid, info);
	if (info == RIONET_DOORBELL_JOIN) {
		if (!rionet_active[sid]) {
			list_for_each_entry(peer, &rionet_peers, node) {
				if (peer->rdev->destid == sid) {
					rionet_active[sid] = peer->rdev;
					nact++;
				}
			}
			rio_mport_send_doorbell(mport, sid,
						RIONET_DOORBELL_JOIN);
		}
	} else if (info == RIONET_DOORBELL_LEAVE) {
		rionet_active[sid] = NULL;
		nact--;
	} else {
		if (netif_msg_intr(rnet))
			printk(KERN_WARNING "%s: unhandled doorbell\n",
			       DRV_NAME);
	}
}

static void rionet_inb_msg_event(struct rio_mport *mport, void *dev_id, int mbox, int slot)
{
	int n;
	struct net_device *ndev = dev_id;
	struct rionet_private *rnet = netdev_priv(ndev);

	if (netif_msg_intr(rnet))
		printk(KERN_INFO "%s: inbound message event, mbox %d slot %d\n",
		       DRV_NAME, mbox, slot);

	spin_lock(&rnet->lock);
	if ((n = rionet_rx_clean(ndev)) != rnet->rx_slot)
		rionet_rx_fill(ndev, n);
	spin_unlock(&rnet->lock);
}

static void rionet_outb_msg_event(struct rio_mport *mport, void *dev_id, int mbox, int slot)
{
	struct net_device *ndev = dev_id;
	struct rionet_private *rnet = netdev_priv(ndev);

	spin_lock(&rnet->lock);

	if (netif_msg_intr(rnet))
		printk(KERN_INFO
		       "%s: outbound message event, mbox %d slot %d\n",
		       DRV_NAME, mbox, slot);

	while (rnet->tx_cnt && (rnet->ack_slot != slot)) {
		/* dma unmap single */
		dev_kfree_skb_irq(rnet->tx_skb[rnet->ack_slot]);
		rnet->tx_skb[rnet->ack_slot] = NULL;
		++rnet->ack_slot;
		rnet->ack_slot &= (RIONET_TX_RING_SIZE - 1);
		rnet->tx_cnt--;
	}

	if (rnet->tx_cnt < RIONET_TX_RING_SIZE)
		netif_wake_queue(ndev);

	spin_unlock(&rnet->lock);
}

static int rionet_open(struct net_device *ndev)
{
	int i, rc = 0;
	struct rionet_peer *peer, *tmp;
	struct rionet_private *rnet = netdev_priv(ndev);

	if (netif_msg_ifup(rnet))
		printk(KERN_INFO "%s: open\n", DRV_NAME);

	if ((rc = rio_request_inb_dbell(rnet->mport,
					(void *)ndev,
					RIONET_DOORBELL_JOIN,
					RIONET_DOORBELL_LEAVE,
					rionet_dbell_event)) < 0)
		goto out;

	if ((rc = rio_request_inb_mbox(rnet->mport,
				       (void *)ndev,
				       RIONET_MAILBOX,
				       RIONET_RX_RING_SIZE,
				       rionet_inb_msg_event)) < 0)
		goto out;

	if ((rc = rio_request_outb_mbox(rnet->mport,
					(void *)ndev,
					RIONET_MAILBOX,
					RIONET_TX_RING_SIZE,
					rionet_outb_msg_event)) < 0)
		goto out;

	/* Initialize inbound message ring */
	for (i = 0; i < RIONET_RX_RING_SIZE; i++)
		rnet->rx_skb[i] = NULL;
	rnet->rx_slot = 0;
	rionet_rx_fill(ndev, 0);

	rnet->tx_slot = 0;
	rnet->tx_cnt = 0;
	rnet->ack_slot = 0;

	netif_carrier_on(ndev);
	netif_start_queue(ndev);

	list_for_each_entry_safe(peer, tmp, &rionet_peers, node) {
		if (!(peer->res = rio_request_outb_dbell(peer->rdev,
							 RIONET_DOORBELL_JOIN,
							 RIONET_DOORBELL_LEAVE)))
		{
			printk(KERN_ERR "%s: error requesting doorbells\n",
			       DRV_NAME);
			continue;
		}

		/* Send a join message */
		rio_send_doorbell(peer->rdev, RIONET_DOORBELL_JOIN);
	}

      out:
	return rc;
}

static int rionet_close(struct net_device *ndev)
{
	struct rionet_private *rnet = netdev_priv(ndev);
	struct rionet_peer *peer, *tmp;
	int i;

	if (netif_msg_ifup(rnet))
		printk(KERN_INFO "%s: close\n", DRV_NAME);

	netif_stop_queue(ndev);
	netif_carrier_off(ndev);

	for (i = 0; i < RIONET_RX_RING_SIZE; i++)
		kfree_skb(rnet->rx_skb[i]);

	list_for_each_entry_safe(peer, tmp, &rionet_peers, node) {
		if (rionet_active[peer->rdev->destid]) {
			rio_send_doorbell(peer->rdev, RIONET_DOORBELL_LEAVE);
			rionet_active[peer->rdev->destid] = NULL;
		}
		rio_release_outb_dbell(peer->rdev, peer->res);
	}

	rio_release_inb_dbell(rnet->mport, RIONET_DOORBELL_JOIN,
			      RIONET_DOORBELL_LEAVE);
	rio_release_inb_mbox(rnet->mport, RIONET_MAILBOX);
	rio_release_outb_mbox(rnet->mport, RIONET_MAILBOX);

	return 0;
}

static void rionet_remove(struct rio_dev *rdev)
{
	struct net_device *ndev = rio_get_drvdata(rdev);
	struct rionet_peer *peer, *tmp;

	free_pages((unsigned long)rionet_active, get_order(sizeof(void *) *
			RIO_MAX_ROUTE_ENTRIES(rdev->net->hport->sys_size)));
	unregister_netdev(ndev);
	free_netdev(ndev);

	list_for_each_entry_safe(peer, tmp, &rionet_peers, node) {
		list_del(&peer->node);
		kfree(peer);
	}
}

static void rionet_get_drvinfo(struct net_device *ndev,
			       struct ethtool_drvinfo *info)
{
	struct rionet_private *rnet = netdev_priv(ndev);

	strcpy(info->driver, DRV_NAME);
	strcpy(info->version, DRV_VERSION);
	strcpy(info->fw_version, "n/a");
	strcpy(info->bus_info, rnet->mport->name);
}

static u32 rionet_get_msglevel(struct net_device *ndev)
{
	struct rionet_private *rnet = netdev_priv(ndev);

	return rnet->msg_enable;
}

static void rionet_set_msglevel(struct net_device *ndev, u32 value)
{
	struct rionet_private *rnet = netdev_priv(ndev);

	rnet->msg_enable = value;
}

static const struct ethtool_ops rionet_ethtool_ops = {
	.get_drvinfo = rionet_get_drvinfo,
	.get_msglevel = rionet_get_msglevel,
	.set_msglevel = rionet_set_msglevel,
	.get_link = ethtool_op_get_link,
};

static const struct net_device_ops rionet_netdev_ops = {
	.ndo_open		= rionet_open,
	.ndo_stop		= rionet_close,
	.ndo_start_xmit		= rionet_start_xmit,
	.ndo_change_mtu		= eth_change_mtu,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_set_mac_address	= eth_mac_addr,
};

static int rionet_setup_netdev(struct rio_mport *mport, struct net_device *ndev)
{
	int rc = 0;
	struct rionet_private *rnet;
	u16 device_id;
	const size_t rionet_active_bytes = sizeof(void *) *
				RIO_MAX_ROUTE_ENTRIES(mport->sys_size);

	rionet_active = (struct rio_dev **)__get_free_pages(GFP_KERNEL,
			get_order(rionet_active_bytes));
	if (!rionet_active) {
		rc = -ENOMEM;
		goto out;
	}
	memset((void *)rionet_active, 0, rionet_active_bytes);

	/* Set up private area */
	rnet = netdev_priv(ndev);
	rnet->mport = mport;

	/* Set the default MAC address */
	device_id = rio_local_get_device_id(mport);
	ndev->dev_addr[0] = 0x00;
	ndev->dev_addr[1] = 0x01;
	ndev->dev_addr[2] = 0x00;
	ndev->dev_addr[3] = 0x01;
	ndev->dev_addr[4] = device_id >> 8;
	ndev->dev_addr[5] = device_id & 0xff;

	ndev->netdev_ops = &rionet_netdev_ops;
	ndev->mtu = RIO_MAX_MSG_SIZE - 14;
	ndev->features = NETIF_F_LLTX;
	SET_ETHTOOL_OPS(ndev, &rionet_ethtool_ops);

	spin_lock_init(&rnet->lock);
	spin_lock_init(&rnet->tx_lock);

	rnet->msg_enable = RIONET_DEFAULT_MSGLEVEL;

	rc = register_netdev(ndev);
	if (rc != 0)
		goto out;

	printk("%s: %s %s Version %s, MAC %pM\n",
	       ndev->name,
	       DRV_NAME,
	       DRV_DESC,
	       DRV_VERSION,
	       ndev->dev_addr);

      out:
	return rc;
}

/*
 * XXX Make multi-net safe
 */
static int rionet_probe(struct rio_dev *rdev, const struct rio_device_id *id)
{
	int rc = -ENODEV;
	u32 lsrc_ops, ldst_ops;
	struct rionet_peer *peer;
	struct net_device *ndev = NULL;

	/* If local device is not rionet capable, give up quickly */
	if (!rionet_capable)
		goto out;

	/* Allocate our net_device structure */
	ndev = alloc_etherdev(sizeof(struct rionet_private));
	if (ndev == NULL) {
		rc = -ENOMEM;
		goto out;
	}

	/*
	 * First time through, make sure local device is rionet
	 * capable, setup netdev,  and set flags so this is skipped
	 * on later probes
	 */
	if (!rionet_check) {
		rio_local_read_config_32(rdev->net->hport, RIO_SRC_OPS_CAR,
					 &lsrc_ops);
		rio_local_read_config_32(rdev->net->hport, RIO_DST_OPS_CAR,
					 &ldst_ops);
		if (!is_rionet_capable(lsrc_ops, ldst_ops)) {
			printk(KERN_ERR
			       "%s: local device is not network capable\n",
			       DRV_NAME);
			rionet_check = 1;
			rionet_capable = 0;
			goto out;
		}

		rc = rionet_setup_netdev(rdev->net->hport, ndev);
		rionet_check = 1;
		nact = 0;
	}

	/*
	 * If the remote device has mailbox/doorbell capabilities,
	 * add it to the peer list.
	 */
	if (dev_rionet_capable(rdev)) {
		if (!(peer = kmalloc(sizeof(struct rionet_peer), GFP_KERNEL))) {
			rc = -ENOMEM;
			goto out;
		}
		peer->rdev = rdev;
		list_add_tail(&peer->node, &rionet_peers);
	}

	rio_set_drvdata(rdev, ndev);

      out:
	return rc;
}

static struct rio_device_id rionet_id_table[] = {
	{RIO_DEVICE(RIO_ANY_ID, RIO_ANY_ID)}
};

static struct rio_driver rionet_driver = {
	.name = "rionet",
	.id_table = rionet_id_table,
	.probe = rionet_probe,
	.remove = rionet_remove,
};

static int __init rionet_init(void)
{
	return rio_register_driver(&rionet_driver);
}

static void __exit rionet_exit(void)
{
	rio_unregister_driver(&rionet_driver);
}

late_initcall(rionet_init);
module_exit(rionet_exit);

#if 1  //jg.xu
static int wan_major = 0;
static struct class *wan_class;
#include <linux/fs.h>
#include <asm/uaccess.h>
static int wan_open(struct inode *inode, struct file *file)
{
	int ret = 0;
	return ret;
}
static int wan_release(struct inode *inode, struct file *file)
{
        return 0;
}
static ssize_t wan_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	unsigned char bin_content_ascii[20]={0};

	
	if(*ppos>=ETH_ALEN*2)
		return 0;

	
	*ppos = ETH_ALEN*2+1;
	
	 if (copy_to_user(buf, bin_content_ascii, sizeof(bin_content_ascii)))
		return -1;
	   
       return  (ETH_ALEN*2+1);
}
#include <linux/vmalloc.h>

static void test_mem(void)
{
	u64 size=90*1024*1024;
	u64 start,end;
	void *data,*data1;
	data=vmalloc(size);
	data1=vmalloc(size);
	if(!data)
		{
		printk("malloc fail\n");
		return;
		}
	memset((u8*)data,0xff,size);
	start=jiffies;
	memcpy((u8*)data1,(u8*)data,size);
	end=jiffies;
	printk("Data copy time %d msec\n",jiffies_to_msecs(end-start));
	if(!memcmp((u8*)data,(u8*)data1,size))
		printk("Data copy correct.\n");
	else
		printk("Data copy fail....\n");
	vfree(data);
	vfree(data1);
}
static void test_disk(void)
{
	printk("jg test_disk\n");
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  format_space
 *  Description:格式化输入字符串，过滤连续的空格为单个空格
 *  Param      : *str  [INOUT]
 *  Return     :
 * =====================================================================================
 */

void format_space(unsigned char *str)
{
#define IN 0            /* 位于空格串中间*/
#define OUT 1           /* 位于空格串外面*/
    unsigned char status=IN;/*过滤开头部分连续的字符串*/
    unsigned char *pos =NULL;
	unsigned char *src = str;
    int i=0;
			
    while(*str != '\0')
    {
        if(' ' == *str)
        {
            if(status == IN)
            {
                *str = '\0';
                pos = str;
                while(' ' == *(str+1))
                {
                    str++;
                }
                i = 0;
                while (str[i+1] != '\0')
                {
                    pos[i] = str[i+1];
                    i++;
                }
                pos[i] = '\0';
                continue;
            }
            else
            {
                status = IN;
            }
        }
        else
        {
            status = OUT;
        }
        str++;
    }

	src[strlen(src)-1] = (src[strlen(src)-1] == ' ' ? '\0' : src[strlen(src)-1]);/*把尾巴的空格去掉*/

    return ;
}		/* -----  end of function format_space  ----- */


extern void read_memory_map(void);
extern void test_riooutb(unsigned char * str);
extern void test_rioinb(unsigned char * str);
extern void test_memory_map(void);
extern void disc_rio(void);
extern void test_doorbell(unsigned char * str);
extern void set_inb_target(unsigned char * str);
extern void testOutBoundSendMsg(unsigned char * str);
extern  void testInBoundGetMsg(unsigned char * str);
static void test_rapidio(void)
{
	printk("jg test_rapidio\n");
	disc_rio();
}
/*housir: added by housir 内联的不能extern? */
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
static inline unsigned long str2hex(unsigned char *str)
{
	int value = 0;
	while (*str) {
		value = value << 4;
		value |= str2hexnum(*str++);
	}

	return value;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  display
 *  Description:  显示指定地址的值
 * =====================================================================================
 */
static  void dm (unsigned char *str)
{
    u32 addr,size,len;
	unsigned char *p=str;
	volatile unsigned char *src=NULL;   
    u32 i;

    printk("housir dm %s\n",str);

	
	p=strsep((char**)&str,(char*)" ");
	addr=str2hex(p);

//	p=strsep((char**)&str,(char*)" ");
//	size=str2hex(p);

	if(str=='\0')
	{
		printk("housir argument error!\n");
		return;
	}

	len=strlen(str);
	str[len-1]='\0';
	
    size = str2hex(str);

    src = (unsigned char *)ioremap(addr, size);

    if (NULL == src)
    {
        printk("==>[%s] remap error\n", __func__);
        return;
    }

    /*housir: display addr */
    for (i=0;i<size;i++)
    {
        if (i%16 == 0)
        {
            printk("\n0x%4.4x:    ", addr+i);
        }
        printk("0x%x ", *(src+i));
    }
    printk("\n");

    iounmap(src);
    
    return ;
}		/* -----  end of static function display  ----- */

static ssize_t wan_write(struct file *file,const char __user *buf, size_t count, loff_t *ppos)
{
	unsigned char bin_content_ascii[100] = {'\0'};

    if(copy_from_user(bin_content_ascii,buf,count))
		return 0;

	printk("jg write %s,%d\n",bin_content_ascii, count);
    *ppos += count;

	
	format_space(bin_content_ascii);

    if(NULL != mem_mport)
    {
        printk("==>[%s] host_deviceid : [%d]\n", __func__, mem_mport->host_deviceid);
    }

	if(!memcmp(bin_content_ascii,"meminfo",sizeof("meminfo")-1))
	{
		test_mem();
	}
	else if(!memcmp(bin_content_ascii,"diskinfo",sizeof("diskinfo")-1))
	{
		test_disk();	
	}
	else if(!memcmp(bin_content_ascii,"rapidio",sizeof("rapidio")-1))
	{
		test_rapidio();
	}
	else if(!memcmp(bin_content_ascii,"setinb",sizeof("setinb")-1))
	{
		u32 length=sizeof("setinb");
		test_rioinb(&bin_content_ascii[length]);
	}
	else if(!memcmp(bin_content_ascii,"setoutb",sizeof("setoutb")-1))
	{
		u32 length=sizeof("setoutb");
		test_riooutb(&bin_content_ascii[length]);
	}
	else if(!memcmp(bin_content_ascii,"writemem",sizeof("writemem")-1))
	{
		test_memory_map();
	}
	else if(!memcmp(bin_content_ascii,"readmem",sizeof("readmem")-1))
	{
		read_memory_map();
	}
	else if(!memcmp(bin_content_ascii,"doorbell",sizeof("doorbell")-1))
	{
		u32 length=sizeof("doorbell");
		test_doorbell(&bin_content_ascii[length]);
	}
	else if(!memcmp(bin_content_ascii,"settarget",sizeof("settarget")-1))
	{
		u32 length=sizeof("settarget");
		set_inb_target(&bin_content_ascii[length]);
	}
	else if(!memcmp(bin_content_ascii,"sendmsg",sizeof("sendmsg")-1))
	{
		u32 length=sizeof("sendmsg");
		testOutBoundSendMsg(&bin_content_ascii[length]);
	}
	else if(!memcmp(bin_content_ascii,"getmsg",sizeof("getmsg")-1))
	{
		u32 length=sizeof("getmsg");
		testInBoundGetMsg(&bin_content_ascii[length]);
	}
	else if(!memcmp(bin_content_ascii,"rioNread",sizeof("rioNread")-1))
	{
		u32 length=sizeof("rioNread");
		test_nread(&bin_content_ascii[length]);
	}
	else if(!memcmp(bin_content_ascii,"rioWrite",sizeof("rioWrite")-1))
	{
		u32 length=sizeof("rioWrite");
		test_nwrite(&bin_content_ascii[length]);
	}
    else if(!memcmp(bin_content_ascii,"dm",sizeof("dm")-1))
	{
		u32 length=sizeof("dm");
		dm(&bin_content_ascii[length]);
	}
    return count;
}
static const struct file_operations wan_fops = {
	.owner		= THIS_MODULE,
	.write		= wan_write,
	.read		= wan_read,
	.open		= wan_open,
	.release	= wan_release,
};

static int __init wan_xjg_init()
{
	int err = 0;

    /*housir: modified by housir */
//	printk("rick wan_wnc_init\n");
	printk(KERN_INFO "rick wan_wnc_init\n");

	wan_major = register_chrdev(0, "wan_major", &wan_fops);
		if (!wan_major) {
			printk(KERN_WARNING "cosa: unable to get major %d\n",
				wan_major);
			err = -EIO;
			goto out;
		}

	wan_class = class_create(THIS_MODULE, "wan_major");
	if (IS_ERR(wan_class)) {
		printk("rick class error\n");
		err = PTR_ERR(wan_class);
		goto out_chrdev;
	}
	device_create(wan_class, NULL, MKDEV(wan_major, 0), NULL,
			      "wan_major%d", 0);
	err = 0;
	goto out;

out_chrdev:
	printk("wan_wnc_init::register failure!!\n");
	unregister_chrdev(wan_major, "wan_major");
out:
    /*housir: modified by housir */
//	printk("wan_wnc_init::register completed!!\n");
	printk(KERN_INFO "wan_wnc_init::register completed!!\n");
	return err;

}
static void __exit wan_xjg_exit()
{
	return;
}

late_initcall(wan_xjg_init);
module_exit(wan_xjg_exit);


#include <linux/pci.h>

#define PCI_NAME "pci_test"
DEFINE_PCI_DEVICE_TABLE(pci_test_ids) = {
	{ PCI_DEVICE(0x10ee,	0x0050), },
	{0}
};

MODULE_DEVICE_TABLE(pci, pci_test_ids);
extern void set_rioinb(u32 localAddr, u32 srioAddr, u32 size,
                  u32 uiInb);
static int pci_test_probe (struct pci_dev *pdev, const struct pci_device_id *id)
{
	int err;
	int retval = -ENXIO;
	unsigned long mem_addr, mem_len;

	err = pci_enable_device(pdev);
	if (err) {
		printk("can't enable pci test device %d\n", err);
		return err;
	}

	retval = pci_request_regions(pdev, PCI_NAME);
	if (retval) {
		printk("can't get pci test region\n");;
		goto out_disable_device;
	}

	//pci_set_master(pdev);
	
	mem_addr = pci_resource_start(pdev, 0);
	mem_len = pci_resource_len(pdev, 0);

	printk("mem_addr %x, mem_len %x\n",mem_addr,mem_len);

	//if (!request_mem_region(mem_addr, mem_len, PCI_NAME))
		//return -EBUSY;
	
	set_rioinb((u32)mem_addr,0x40000000,(u32)mem_len,0);
	
	return 0;
out_disable_device:
	pci_disable_device(pdev);
	return EBUSY;

}

static void pci_test_remove (struct pci_dev *pdev)
{
	pci_disable_device(pdev);
}

static struct pci_driver pci_test_driver = {
	.name = PCI_NAME,
	.id_table = pci_test_ids,
	.probe = pci_test_probe,
	.remove = pci_test_remove,
};

static int __init  pci_test_init()
{
	return pci_register_driver(&pci_test_driver);
}
static void __exit pci_test_exit()
{
	pci_unregister_driver (&pci_test_driver);
}

late_initcall(pci_test_init);
module_exit(pci_test_exit);

#endif

