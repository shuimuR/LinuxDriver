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

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>

static struct class *FirstDrvClass;
static struct class_device *FirstDrvClassDevs;

static int FirstOpen(struct inode *inode, struct file *file)
{
	printk("FirstOpen\n");
	return 0;	
}

static ssize_t FirstWrite(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	printk("FirstWrite\n");
	return 0;	
}

static struct file_operations FirstStructOps = 
{	
	.owner = THIS_MODULE,
	.open = FirstOpen,
	.write = FirstWrite,	
};

int major; 

int FirstDrvInit()
{
	printk("FirstDrvInit\n");
	major = register_chrdev(0, "FirstDrv", &FirstStructOps);
 	
	FirstDrvClass = class_create(THIS_MODULE, "FirstDrv");
	if(IS_ERR(FirstDrvClass))
		return PTR_ERR(FirstDrvClass);
	
	FirstDrvClassDevs = class_device_create(FirstDrvClass, NULL, MKDEV(major, 0), NULL, "xyz");			//创建类FirstDrv下的/dev/xyz节点
	if(unlikely(IS_ERR(FirstDrvClassDevs)))
		return PTR_ERR(FirstDrvClassDevs);

	return 0;	
}

void FirstExit()
{
	printk("FirstExit\n");
	unregister_chrdev(major, "FirstDrv");	
	class_device_unregister(FirstDrvClassDevs);
	class_destroy(FirstDrvClass);
}

module_init(FirstDrvInit);
module_exit(FirstExit);

MODULE_LICENSE("GPL");
