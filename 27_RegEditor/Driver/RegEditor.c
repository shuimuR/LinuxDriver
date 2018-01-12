#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/irq.h>
#include <linux/wait.h>
#include <linux/poll.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>

#define KER_RW_R8   0
#define KER_RW_R16   1
#define KER_RW_R32   2

#define KER_RW_W8   3
#define KER_RW_W16   4
#define KER_RW_W32   5

static int major;

static struct class *RegEditorClass;
static struct class_device *RegEditorDev;

static int RegEditIOCtrl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
    volatile unsigned char *p8 = NULL;
    volatile unsigned short *p16 = NULL;
    volatile unsigned int *p32 = NULL;

    unsigned int val = 0;
    unsigned int addr;

    unsigned int buf[2];

    copy_from_user(buf, (const void __user *)arg, 8);
    addr = buf[0];
    val = buf[1];

    p8 = (volatile unsigned char *)ioremap(addr, 4);
    p16 = (volatile unsigned short *)p8;
    p32 = (volatile unsigned int *)p8;

    switch(cmd)
    {
        case KER_RW_R8:
            val = *p8;
            copy_to_user((void __user *)(arg + 4), &val, 4);
            break;
        case KER_RW_R16:
            val = *p16;
            copy_to_user((void __user *)(arg + 4), &val, 4);
            break;
        case KER_RW_R32:
            val = *p32;
            copy_to_user((void __user *)(arg + 4), &val, 4);
            break;

        case KER_RW_W8:
            *p8 = val;
            break;
        case KER_RW_W16:
            *p16 = val;
            break;
        case KER_RW_W32:
            *p32 = val;
            break;
    }

    iounmap(p8);
    return 0;
}

static struct file_operations RegEditOps =
{
    .owner = THIS_MODULE,
    .ioctl = RegEditIOCtrl,
};

static int RegEditorInit(void)
{
    major = register_chrdev(0, "RegEdit", &RegEditOps);
	RegEditorClass = class_create(THIS_MODULE, "RegEdit");

	RegEditorDev = class_device_create(RegEditorClass, NULL, MKDEV(major, 0), NULL, "RegEdit");			//创建类FirstDrv下的/dev/xyz节点

	printk("RegEditorInit\n");
	return 0;
}

static void RegEditorExit(void)
{
    unregister_chrdev(major, "RegEdit");
	class_device_unregister(RegEditorDev);
	class_destroy(RegEditorClass);
}

module_init(RegEditorInit);
module_exit(RegEditorExit);

MODULE_LICENSE("GPL");
