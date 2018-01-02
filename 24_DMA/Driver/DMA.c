#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <linux/poll.h>
#include <linux/dma-mapping.h>

#define MEM_CPY_NO_DMA  0
#define MEM_CPY_DMA     1

static int major;

static char *src;
static u32 src_phys;

static char *dst;
static u32 dst_phys;

static DECLARE_WAIT_QUEUE_HEAD(DMA_waitq);
static volatile int ev_dma = 0;

#define BUF_SIZE (512*1024)

static struct class *DmaClass;
static struct class_device *DmaDev;

#define DMA0_BASE_ADDR  0x4B000000
#define DMA1_BASE_ADDR  0x4B000040
#define DMA2_BASE_ADDR  0x4B000080
#define DMA3_BASE_ADDR  0x4B0000C0

struct s3c_dma_regs {
    unsigned long disrc;
    unsigned long disrcc;
    unsigned long didst;
    unsigned long didstc;
    unsigned long dcon;
    unsigned long dstat;
    unsigned long dcsrc;
    unsigned long dcdst;
    unsigned long dmasktrig;
};

static volatile struct s3c_dma_regs *DMA_Regs;

static int DMA_IO_Ctrl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
    memset(src, 0xAA, BUF_SIZE);
    memset(dst, 0x55, BUF_SIZE);
    switch(cmd)
    {
        case MEM_CPY_NO_DMA:
        {
            int i;
            for(i = 0; i < BUF_SIZE; i++)
                dst[i] = src[i];

            if(memcmp(src, dst, BUF_SIZE) == 0)
                printk("NO DMA src = dst\n");
            else
                printk("NO DMA src != dst\n");
            break;
        }
        case MEM_CPY_DMA:
        {
            DMA_Regs = ioremap(DMA3_BASE_ADDR, sizeof(struct s3c_dma_regs));

            DMA_Regs->disrc = src_phys;
            DMA_Regs->disrcc = 0;

            DMA_Regs->didst = dst_phys;
            DMA_Regs->didstc = (0 << 2) | (0 << 1) | (0 << 0);

            DMA_Regs->dcon = (1 << 30) | (1 << 29) | (1 << 27) | BUF_SIZE;

            //启动dma
            DMA_Regs->dmasktrig = (1 << 1) | (1 << 0);

            //开始休眠  在中断中唤醒
            wait_event_interruptible(DMA_waitq, ev_dma);

            if(memcmp(src, dst, BUF_SIZE) == 0)
                printk("use DMA src = dst\n");
            else
                printk("use DMA src != dst\n");
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

static irqreturn_t DMA_Irq(int irq, void *devid)
{
    ev_dma = 1;
    wake_up_interruptible(&DMA_waitq);
    return IRQ_HANDLED;
}


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

    //注册DMA 中断
    if(request_irq(IRQ_DMA3, DMA_Irq, 0, "DMA_Irq", 1))
    {
        printk("can't request irq for DMA\n");
        unregister_chrdev(major, "DMA");
        class_device_unregister(DmaDev);
        class_destroy(DmaClass);
        dma_free_writecombine(NULL, BUF_SIZE, src, src_phys);
        dma_free_writecombine(NULL, BUF_SIZE, dst, dst_phys);
        return -EBUSY;
    }

    return 0;
}

static void DMA_Exit(void)
{
    free_irq(IRQ_DMA3, 1);
    iounmap(DMA_Regs);
    unregister_chrdev(major, "DMA");
    class_device_unregister(DmaDev);
    class_destroy(DmaClass);
    dma_free_writecombine(NULL, BUF_SIZE, src, src_phys);
    dma_free_writecombine(NULL, BUF_SIZE, dst, dst_phys);
}

module_init(DMA_Init);
module_exit(DMA_Exit);

MODULE_LICENSE("GPL");