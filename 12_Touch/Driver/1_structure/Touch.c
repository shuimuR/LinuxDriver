#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <asm/irq.h>

#include <asm/plat-s3c24xx/ts.h>

#include <asm/arch/regs-adc.h>
#include <asm/arch/regs-gpio.h>

static struct input_dev *TouchDev;

static int TouchInit()
{
    //分配一个input_dev结构体
    TouchDev = input_allocate_device();
    //设置
    //1. 能产生哪类事件
    set_bit(EV_KEY, TouchDev->evbit);
    set_bit(EV_ABS, TouchDev->evbit);

    //2. 能产生这类事件中的哪些事件
    set_bit(BTN_TOUCH, TouchDev->keybit);

    input_set_abs_params(TouchDev, ABS_X, 0, 0x3FF, 0, 0);
    input_set_abs_params(TouchDev, ABS_Y, 0, 0x3FF, 0, 0);
    input_set_abs_params(TouchDev, ABS_PRESSURE, 0, 1, 0, 0);

    //注册

    //硬件相关的操作
    input_register_device(TouchDev);

    return 0;
}

static void TouchExit()
{

}

module_init(TouchInit);
module_exit(TouchExit);

MODULE_LICENSE("GPL");