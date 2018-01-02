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

static int major;

static char *src;
static u32 src_phys;

static char *dst;
static u32 dst_phys;

#define BUF_SIZE (512*1024)

static struct class *DmaClass;
static struct class_device *DmaDev;


static int DMA_IO_Ctrl(struct inode *inode, struct file *file, )
{
    switch(cmd)
    {
        case MEM_CPY_NO_DMA:
        {
            break;
        }
        case MEM_CPY_DMA:
        {
            break;
        }
    }
    return 0;
}

static struct file_operations DMA_Fops =
        {
                .owner = THIS_MODULE,
                .ioctl = DMA_IO_Ctrl,
        };

static int DMA_Init(void)
{

    //分配src dst对应的缓冲区，不能用malloc，因为如果用malloc分配
    //可能得到物理地址不连续的存储空间
    src = dma_alloc_writecombine(NULL, BUF_SIZE, &src_phys, GFP_KERNEL);
    if(src == NULL)
    {
        printk("can't alloc src memory\n");
        return -ENOMEM;
    }

    dst = dma_alloc_writecombine(NULL, BUF_SIZE, &dst_phys, GFP_KERNEL);
    if(dst == NULL)
    {
        printk("can't alloc dst memory\n");
        dma_free_writecombine(NULL, BUF_SIZE, src, src_phys);
        return -ENOMEM;
    }

    major = register_chrdev(0, "DMA", &DMA_Fops);

    DmaClass = class_create(THIS_MODULE, "DMA");

    DmaDev = class_device_create(DmaClass, NULL, MKDEV(major, 0), NULL, "DMA");			//创建类FirstDrv下的/dev/xyz节点

    return 0;
}

static void DMA_Exit(void)
{
    unregister_chrdev(major, "DMA");
    class_device_unregister(DmaDev);
    class_destroy(DmaClass);
    dma_free_writecombine(NULL, BUF_SIZE, src, src_phys);
    dma_free_writecombine(NULL, BUF_SIZE, dst, dst_phys)
}

module_init(DMA_Init);
module_exit(DMA_Exit);

MODULE_LICENSE("GPL");