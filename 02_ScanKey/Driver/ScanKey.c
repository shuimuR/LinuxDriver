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


#include <asm/io.h>
#include <asm/irq.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>

static struct class *ScanKeyClass;
static struct class_device *ScanKeyDev;

volatile unsigned long *gpfcon = NULL;
volatile unsigned long *gpfdat = NULL;
volatile unsigned long *gpgcon = NULL;
volatile unsigned long *gpgdat = NULL;



static int  ScanKeyOpen(struct inode *inode, struct file *file)
{
	printk("file open\n");
	*gpfcon &= ~((0x03 << (0*2)) | (0x03 << (2 * 2)));
	*gpgcon &= ~((0x03 << (3 * 2)) | (0x03 << (11*2)));
	return 0;	
}

static ssize_t ScanKeyRead(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	unsigned char KeyVal[4];
	int regval;

	if(size != sizeof(KeyVal))
	{
		printk("There are 4 keys, so you must read 4 datas\n");
		return -1;	
	}

	regval = *gpfdat;
	KeyVal[0] = (regval & (1 << 0) ? 1 : 0);
	KeyVal[1] = (regval & (1 << 2) ? 1 : 0);

	regval = *gpgdat;
	KeyVal[2] = (regval & (1 << 3) ? 1 : 0);
	KeyVal[3] = (regval & (1 << 11) ? 1 : 0);
	
	copy_to_user(buf, KeyVal, sizeof(KeyVal));

	printk("KeyScan read");

	return 0;	
}

static struct file_operations ScanKeyOperations = 
{
	.owner = THIS_MODULE,
	.open = ScanKeyOpen,
	.read = ScanKeyRead,	
};

int major = 0;

static int ScanKeyInit()
{
	major = register_chrdev(0, "ScanKey", &ScanKeyOperations);
	ScanKeyClass = class_create(THIS_MODULE, "ScanKey");
	if(IS_ERR(ScanKeyClass))
		return PTR_ERR(ScanKeyClass);
					
	ScanKeyDev = class_device_create(ScanKeyClass, NULL, MKDEV(major, 0), NULL, "key");			//创建类FirstDrv下的/dev/xyz节点
	if(unlikely(IS_ERR(ScanKeyDev)))
		return PTR_ERR(ScanKeyDev);
	
	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;			//driver use map addr
	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);
	gpgdat = gpgcon + 1;			//driver use map addr
	
	printk("ScanKey init\n");
	return 0;	
}

static void ScanKeyExit()
{
	unregister_chrdev(major, "ScanKey");		
	class_device_unregister(ScanKeyDev);
	class_destroy(ScanKeyClass);
	
	iounmap(gpfcon);
	iounmap(gpgcon);
	printk("ScanKey exit\n");
}

module_init(ScanKeyInit);
module_exit(ScanKeyExit);

MODULE_LICENSE("GPL");

