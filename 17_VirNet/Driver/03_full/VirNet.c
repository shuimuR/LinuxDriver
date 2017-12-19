/***************************************
参考：drivers/net/cs89x0.c
 **************************************/

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/ip.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/irq.h>

static struct net_device *VirDev;

static void emulate_rx_packet(struct sk_buff *skb, struct net_device *dev)
{
    /* 参考LDD3 */
    unsigned char *type;
    struct iphdr *ih;
    __be32 *saddr, *daddr, tmp;
    unsigned char	tmp_dev_addr[ETH_ALEN];
    struct ethhdr *ethhdr;

    struct sk_buff *rx_skb;

    // 从硬件读出/保存数据
    /* 对调"源/目的"的mac地址 */
    ethhdr = (struct ethhdr *)skb->data;
    memcpy(tmp_dev_addr, ethhdr->h_dest, ETH_ALEN);
    memcpy(ethhdr->h_dest, ethhdr->h_source, ETH_ALEN);
    memcpy(ethhdr->h_source, tmp_dev_addr, ETH_ALEN);

    /* 对调"源/目的"的ip地址 */
    ih = (struct iphdr *)(skb->data + sizeof(struct ethhdr));
    saddr = &ih->saddr;
    daddr = &ih->daddr;

    tmp = *saddr;
    *saddr = *daddr;
    *daddr = tmp;

    //((u8 *)saddr)[2] ^= 1; /* change the third octet (class C) */
    //((u8 *)daddr)[2] ^= 1;
    type = skb->data + sizeof(struct ethhdr) + sizeof(struct iphdr);
    //printk("tx package type = %02x\n", *type);
    // 修改类型, 原来0x8表示ping
    *type = 0; /* 0表示reply */

    ih->check = 0;		   /* and rebuild the checksum (ip needs it) */
    ih->check = ip_fast_csum((unsigned char *)ih,ih->ihl);

    // 构造一个sk_buff
    rx_skb = dev_alloc_skb(skb->len + 2);
    skb_reserve(rx_skb, 2); /* align IP on 16B boundary */
    memcpy(skb_put(rx_skb, skb->len), skb->data, skb->len);

    /* Write metadata, and then pass to the receive level */
    rx_skb->dev = dev;
    rx_skb->protocol = eth_type_trans(rx_skb, dev);
    rx_skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */
    dev->stats.rx_packets++;
    dev->stats.rx_bytes += skb->len;

    // 提交sk_buff
    netif_rx(rx_skb);
}

static int virt_net_sendpacket(struct sk_buff *skb, struct net_device *dev)
{
    static int cnt = 0;

    printk("virt_net_sendpacket cnt = %d\n", ++cnt);

    netif_stop_queue(dev);      //停止该网卡的队列

    //将数据写入网卡

    //构造假的sk_buff, 上报
    emulate_rx_packet(skb, dev);

    //释放skb
    dev_kfree_skb (skb);

    //数据全部发送之后，唤醒网卡队列
    netif_wake_queue(dev);

    dev->stats.tx_bytes += skb->len;
    dev->stats.tx_packets++;

    return 0;
}

static int VirNetInit(void)
{
    //1. 分配net_device结构体
    VirDev = alloc_netdev(0, "vnet%d", ether_setup);

    //2. 设置
    VirDev->hard_start_xmit = virt_net_sendpacket;

    //MAC addr
    VirDev->dev_addr[0] = 0x77;
    VirDev->dev_addr[1] = 0x77;
    VirDev->dev_addr[2] = 0x77;
    VirDev->dev_addr[3] = 0x77;
    VirDev->dev_addr[4] = 0x77;
    VirDev->dev_addr[5] = 0x77;

    /* 设置下面两项才能ping通 */
    VirDev->flags           |= IFF_NOARP;
    VirDev->features        |= NETIF_F_NO_CSUM;


    //3. 注册
    register_netdev(VirDev);

    return 0;
}

static void VirNetExit(void)
{
    unregister_netdev(VirDev);
    free_netdev(VirDev);
}

module_init(VirNetInit);
module_exit(VirNetExit);

MODULE_LICENSE("GPL");
