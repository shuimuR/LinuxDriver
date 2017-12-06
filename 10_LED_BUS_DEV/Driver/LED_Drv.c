/****************************************
分配/设置/注册一个platform_driver
****************************************/
#include <linux/module.h>
#include <linux/version.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/uaccess.h>

#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>

static int major;
//IO remap
static volatile unsigned long *gpio_con;
static volatile unsigned long *gpio_dat;
static int pin;

static int LED_Open(struct inode *inode, struct file *file)
{
    printk("FirstOpen\n");
    *gpio_con &= ~((0x03 << (pin*2)));
    *gpio_con |= (0x01 << (pin * 2));
    return 0;
}

static ssize_t LED_Write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
    int val = 0;

    copy_from_user(&val, buf, count);

    if(val == 1)
    {
        *gpio_dat &= ~(1 << pin);
    }
    else
    {
        *gpio_dat |= (1 << pin);
    }
    printk("LED write %d\n", val);

    return 0;
}

static struct file_operations led_fops =
        {
                .owner = THIS_MODULE,
                .open  = LED_Open,
                .write = LED_Write,
        };

static struct class *LEDDrvClass;
static struct class_device *LEDDrvClassDevs;

static int LED_Probe(struct platform_device *dev)
{
    struct resource *res;
    //根据platform_device资源进行ioremap
    res = platform_get_resource(dev, IORESOURCE_MEM, 0);
    gpio_con = ioremap(res->start, res->end - res->start + 1);
    gpio_dat = gpio_con + 1;

    res = platform_get_resource(dev, IORESOURCE_IRQ, 0);
    pin = res->start;
    //注册字符设备驱动程序

    printk("LED probe\n");

    major = register_chrdev(0, "MyLED", &led_fops);

    LEDDrvClass = class_create(THIS_MODULE, "MyLED");
    if(IS_ERR(LEDDrvClass))
        return PTR_ERR(LEDDrvClass);

    LEDDrvClassDevs = class_device_create(LEDDrvClass, NULL, MKDEV(major, 0), NULL, "LED");			//创建类FirstDrv下的/dev/xyz节点
    if(unlikely(IS_ERR(LEDDrvClassDevs)))
        return PTR_ERR(LEDDrvClassDevs);

    return 0;
}

static void LED_Remove()
{
    printk("LED remove\n");
    unregister_chrdev(major, "MyLED");
    class_device_unregister(LEDDrvClassDevs);
    class_destroy(LEDDrvClass);

    iounmap(gpio_con);
}

static struct platform_driver LED_Drv =
        {
                .probe = LED_Probe,
                .remove = LED_Remove,
                .driver = {
                        .name = "MyLED",
                }
        };


static int LED_DrvInit()
{
    platform_driver_register(&LED_Drv);

    return 0;
}

static void LED_DrvExit()
{
    platform_driver_unregister(&LED_Drv);
}

module_init(LED_DrvInit);

module_exit(LED_DrvExit);

MODULE_LICENSE("GPL");