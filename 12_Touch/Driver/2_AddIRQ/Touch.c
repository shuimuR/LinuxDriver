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

struct TouchRegs
{
    unsigned long ADCCON;
    unsigned long ADCTSC;
    unsigned long ADCDLY;
    unsigned long ADCDAT0;
    unsigned long ADCDAT1;
    unsigned long ADCUPDN;
};

static struct TouchRegs *TouchReg;

static void WaitEnterPenDownMode()
{
    TouchReg->ADCTSC = 0xD3;
}

static void WaitEnterPenUpMode()
{
    TouchReg->ADCTSC = 0x53;
}

static irqreturn_t TouchIRQ(int irq, void *dev_id)
{
    if(TouchReg->ADCDAT0 & (1 << 15))
    {
        printk("Pen up\n");
        WaitEnterPenDownMode();
    }
    else
    {
        printk("Pen down\n");
        WaitEnterPenUpMode();
    }
    return IRQ_HANDLED;
}

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
    input_register_device(TouchDev);

    //硬件相关的操作
    //1. 使能clk
    struct clk * clk;
    clk = clk_get(NULL, "adc");
    clk_enable(clk);

    //2. 设置ADC/TOUCH的寄存器
    TouchReg = ioremap(0x58000000, 32);
    TouchReg->ADCCON = (1 << 14) | (49 << 6);

    request_irq(IRQ_TC, TouchIRQ, IRQF_SAMPLE_RANDOM, "ts_open", NULL);

    TouchReg->ADCTSC = 0xd3;            //enter wait interrupt mode


    return 0;
}

static void TouchExit()
{
    free_irq(IRQ_TC, NULL);
    iounmap(TouchReg);
    input_unregister_device(TouchDev);
    input_free_device(TouchDev);
}

module_init(TouchInit);
module_exit(TouchExit);

MODULE_LICENSE("GPL");