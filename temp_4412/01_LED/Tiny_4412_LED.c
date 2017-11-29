#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/delay.h>

#include <linux/gpio.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>

static struct class *LEDsDrvClass;
static struct class_device *LEDsClassDev;
static int led_gpios[] = 
{
	EXYNOS4212_GPM4(0),
	EXYNOS4212_GPM4(1),	
	EXYNOS4212_GPM4(2),	
	EXYNOS4212_GPM4(3),	
};

static int LEDs_Open(struct inode *inode, struct file *filp)
{
	int i;
	for(i = 0; i < 4; i++)
	{
		 s3c_gpio_cfgpin(led_gpios[i], S3C_GPIO_OUTPUT);	
	}
	printk("LEDs_Open\n");
	return 0;
}

static long LEDs_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	//cmd: 1--on  0--off
	//arg: 0~3
	if(cmd !=0 && cmd != 1)
	{
		printk("args is not 0 or 1\n");
		return -EINVAL;
	}	
	if(arg > 4)
	{
		printk("arg > 4\n");
		return -EINVAL;
	}
	gpio_set_value(led_gpios[arg], !cmd);

	printk("LED set value over\n");
}

static struct file_operations LEDs_Ops = 
{
	.owner = THIS_MODULE,
	.open = LEDs_Open,
	.unlocked_ioctl = LEDs_ioctl,		
};

static int major;

int LEDs_Init(void)
{
	major = register_chrdev(0, "LEDs", &LEDs_Ops);
	LEDsDrvClass = class_create(THIS_MODULE, "LEDs");
	LEDsClassDev = device_create(LEDsDrvClass, NULL, MKDEV(major, 0), NULL, "LEDs");
	
	printk("LED init over\n");
	return 0;	

}

void LEDs_Exit(void)
{
	device_destroy(LEDsDrvClass, MKDEV(major, 0));
	class_destroy(LEDsClassDev);
	unregister_chrdev(major, "LEDs");
	printk("LED exit over\n");
}

module_init(LEDs_Init);
module_init(LEDs_Exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("shuimu"); 
