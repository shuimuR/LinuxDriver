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
#include <linux/cdev.h>


static int major;

static int hello_open(struct inode *inode, struct file *file)
{
    printk("Hello_Open\n");
    return 0;
}

#define HelloCNT    2

static struct file_operations HelloFops =
        {
                .owner = THIS_MODULE,
                .open = hello_open,
        };

static struct cdev hello_dev;

static struct class *HelloClass;
static struct class_device *HelloDev;

static int HelloInit(void)
{
    dev_t devid;

    //只关联次设备号为0,1的两个节点

    if(major)
    {
        devid = MKDEV(major, 0);
        register_chrdev_region(devid, HelloCNT, "Hello");
    }
    else
    {
        alloc_chrdev_region(&devid, 0, HelloCNT, "Hello");
        major = MAJOR(devid);
    }

    cdev_init(&hello_dev, &HelloFops);
    cdev_add(&hello_dev, devid, HelloCNT);

    HelloClass = class_create(THIS_MODULE, "Hello");
    if(IS_ERR(HelloClass))
        return PTR_ERR(HelloClass);

    class_device_create(HelloClass, NULL, MKDEV(major, 0), NULL, "Hello0");
    class_device_create(HelloClass, NULL, MKDEV(major, 1), NULL, "Hello1");
    class_device_create(HelloClass, NULL, MKDEV(major, 2), NULL, "Hello2");

    return 0;
}

static void HelloExit(void)
{
    class_device_destroy(HelloClass, MKDEV(major, 0));
    class_device_destroy(HelloClass, MKDEV(major, 1));
    class_device_destroy(HelloClass, MKDEV(major, 2));
    class_destroy(HelloClass);

    cdev_del(&hello_dev);
    unregister_chrdev_region(MKDEV(major, 0), HelloCNT);
}

module_init(HelloInit);
module_exit(HelloExit);

MODULE_LICENSE("GPL");