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

extern int MyPrintk(const char *fmt, ...);


static int major;

static int hello_open(struct inode *inode, struct file *file)
{
    printk("Hello_Open\n");
    return 0;
}

static int hello2_open(struct inode *inode, struct file *file)
{
    printk("Hello2_Open\n");
    return 0;
}

#define HelloCNT    2

static struct file_operations HelloFops =
        {
                .owner = THIS_MODULE,
                .open = hello_open,
        };

static struct file_operations HelloFops2 =
        {
                .owner = THIS_MODULE,
                .open = hello2_open,
        };

static struct cdev hello_dev;
static struct cdev hello2_dev;

static struct class *HelloClass;
static struct class_device *HelloDev;

static int HelloInit(void)
{
    MyPrintk("HelloInit\n");

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

    //add Hello 2
    devid = MKDEV(major, 2);
    register_chrdev_region(devid, 1, "Hello2");
    cdev_init(&hello2_dev, &HelloFops2);
    cdev_add(&hello2_dev, devid, 1);

    HelloClass = class_create(THIS_MODULE, "Hello");
    if(IS_ERR(HelloClass))
        return PTR_ERR(HelloClass);

    class_device_create(HelloClass, NULL, MKDEV(major, 0), NULL, "Hello0");
    class_device_create(HelloClass, NULL, MKDEV(major, 1), NULL, "Hello1");
    class_device_create(HelloClass, NULL, MKDEV(major, 2), NULL, "Hello2");
    class_device_create(HelloClass, NULL, MKDEV(major, 3), NULL, "Hello3");

    return 0;
}

static void HelloExit(void)
{
    MyPrintk("HelloExit\n");
    class_device_destroy(HelloClass, MKDEV(major, 0));
    class_device_destroy(HelloClass, MKDEV(major, 1));
    class_device_destroy(HelloClass, MKDEV(major, 2));
    class_device_destroy(HelloClass, MKDEV(major, 3));
    class_destroy(HelloClass);

    cdev_del(&hello_dev);
    unregister_chrdev_region(MKDEV(major, 0), HelloCNT);

    cdev_del(&hello2_dev);
    unregister_chrdev_region(MKDEV(major, 2), 1);
}

module_init(HelloInit);
module_exit(HelloExit);

MODULE_LICENSE("GPL");
