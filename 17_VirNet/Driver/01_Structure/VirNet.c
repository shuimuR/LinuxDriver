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

#include <asm/system.h>
#include <asm/io.h>
#include <asm/irq.h>

static struct net_device *VirDev;

static int virt_net_sendpacket(struct sk_buff *skb, struct net_device *dev)
{
    static int cnt = 0;
    printk("virt_net_sendpacket cnt = %d\n", ++cnt);
    return 0;
}

static int VirNetInit(void)
{
    //1. 分配net_device结构体
    VirDev = alloc_netdev(0, "vnet%d", ether_setup);

    //2. 设置
    VirDev->hard_start_xmit = virt_net_sendpacket;

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
