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

static struct class *ScanKeyClass;
static struct class_device *ScanKeyDev;

static struct timer_list ButtonTimer;

volatile unsigned long *gpfcon = NULL;
volatile unsigned long *gpfdat = NULL;
volatile unsigned long *gpgcon = NULL;
volatile unsigned long *gpgdat = NULL;

struct pin_desc
{
	unsigned int pin;
	unsigned int key_val;
};


//press 0x01 0x02 0x03 0x04
//release 0x81 0x82 0x83 0x84
struct pin_desc pindescs[4] = 
{
	{S3C2410_GPF0, 0x01},
	{S3C2410_GPF2, 0x02},
	{S3C2410_GPG3, 0x03},
	{S3C2410_GPG11, 0x04 },
	
};

static DECLARE_MUTEX(ButtonLock);

static struct fasync_struct *button_async;

//to avoid the programe waste the CPU, define the varient
static DECLARE_WAIT_QUEUE_HEAD(button_waitq);
static volatile int ev_press = 0;

static struct pin_desc *irq_pd = NULL;

static unsigned char KeyVal;
/******************************************
确定按键值
******************************************/
static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	printk("irq = %d\n", irq);
	irq_pd = (struct pin_desc *)dev_id;
	mod_timer(&ButtonTimer, jiffies + HZ/100);		//jiffies every 10 ms system timeout 	
	return IRQ_HANDLED;
}

static int  KeyIntrOpen(struct inode *inode, struct file *file)
{
	if(file->f_flags & O_NONBLOCK)
	{
		if(down_trylock(&ButtonLock))
			return -EBUSY;		
	}
	else
	{
		down(&ButtonLock);
	}
		printk("file open\n");
		//config the intr
		request_irq(IRQ_EINT0, buttons_irq, IRQT_BOTHEDGE, "S2", &pindescs[0]);
		request_irq(IRQ_EINT2, buttons_irq, IRQT_BOTHEDGE, "S3", &pindescs[1]);
		request_irq(IRQ_EINT11, buttons_irq, IRQT_BOTHEDGE, "S4",&pindescs[2]);
		request_irq(IRQ_EINT19, buttons_irq, IRQT_BOTHEDGE, "S5",&pindescs[3]);
	
	return 0;
}

static ssize_t KeyIntrRead(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	if(size != 1) 
	{
		printk("You just read a value now\n");
		return -1;	
	}

	if(file->f_flags & O_NONBLOCK)
	{
		if(ev_press == 0)
			return EAGAIN;	
	}
	else
	{
		//if there's no key event
		wait_event_interruptible(button_waitq, ev_press);
	}
	printk("run the copy_to_user\n");
	//there's key event
	copy_to_user(buf, &KeyVal,1); 
	ev_press = 0;
	printk("Clear the flag\n");
	
	return 0;	
}

int KeyIntrClose(struct inode *inode, struct file *file)
{
	up(&ButtonLock);
	free_irq(IRQ_EINT0, &pindescs[0]);	
	free_irq(IRQ_EINT2, &pindescs[1]);	
	free_irq(IRQ_EINT11, &pindescs[2]);	
	free_irq(IRQ_EINT19, &pindescs[3]);	
	return 0;
}

static unsigned KeyIntrPoll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;
	poll_wait(file, &button_waitq, wait);
	if(ev_press)
		mask |= POLLIN | POLLRDNORM;
	return mask;
}

static int ButtonSync(int fd, struct file *filp, int on)
{
	printk("Driver: run ButtonSync to register the fdsync\n");
	return fasync_helper(fd, filp, on, &button_async);	
}

static struct file_operations ScanKeyOperations = 
{
	.owner = THIS_MODULE,
	.open = KeyIntrOpen,
	.read = KeyIntrRead,	
	.release = KeyIntrClose,
	.poll = KeyIntrPoll,
	.fasync = ButtonSync,
};

static void ButtonTimeOut(unsigned long time)
{
	struct pin_desc *CurrentDev= irq_pd; 
	unsigned int pinval;
	
	if(!CurrentDev)
		return;

	pinval = s3c2410_gpio_getpin(CurrentDev->pin);
 	printk("Read the GPIO value\n");	
	if(pinval)
		KeyVal= 0x80 | CurrentDev->key_val;
	else
		KeyVal= CurrentDev->key_val;
	printk("Prepare to wake up read\n");
	ev_press = 1;
	wake_up_interruptible(&button_waitq);
	
	kill_fasync(&button_async, SIGIO, POLL_IN);
}

int major = 0;

static int KeyIntrInit()
{
	major = register_chrdev(0, "KeyIntr", &ScanKeyOperations);
	ScanKeyClass = class_create(THIS_MODULE, "KeyIntr");
	if(IS_ERR(ScanKeyClass))
		return PTR_ERR(ScanKeyClass);
					
	ScanKeyDev = class_device_create(ScanKeyClass, NULL, MKDEV(major, 0), NULL, "key");			//创建类FirstDrv下的/dev/xyz节点
	if(unlikely(IS_ERR(ScanKeyDev)))
		return PTR_ERR(ScanKeyDev);

	init_timer(&ButtonTimer);
	ButtonTimer.function = ButtonTimeOut;
	add_timer(&ButtonTimer);

	printk("KeyIntr init\n");
	return 0;	
}

static void KeyIntrExit()
{
	unregister_chrdev(major, "KeyIntr");		
	class_device_unregister(ScanKeyDev);
	class_destroy(ScanKeyClass);
	
	iounmap(gpfcon);
	iounmap(gpgcon);
	printk("KeyIntr exit\n");
}

module_init(KeyIntrInit);
module_exit(KeyIntrExit);

MODULE_LICENSE("GPL");

