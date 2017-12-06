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

#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>

static struct resource led_resource[] =
        {
                [0] = {
                        .start = 0x56000050,
                        .end = 0x56000050 + 8 - 1,
                        .flags = IORESOURCE_MEM,
                },
                [1] = {
                        .start = 4,
                        .end = 4,
                        .flags = IORESOURCE_IRQ,
                }
        };

static void LED_Release(struct device * dev)
{

}

static struct platform_device led_dev =
        {
                .name = "MyLED",
                .id = -1,
                .num_resources = ARRAY_SIZE(led_resource),
                .resource = led_resource,
                .dev = {
                        .release = LED_Release,
                },
        };


static int LED_DevInit()
{
    platform_device_register(&led_dev);
    return 0;
}

static void LED_DevExit()
{
    platform_device_unregister(&led_dev);
}

module_init(LED_DevInit);

module_exit(LED_DevExit);

MODULE_LICENSE("GPL");