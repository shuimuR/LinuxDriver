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
    TouchReg->ADCTSC = 0x1D3;
}

/*****************************************************
 * 转换x y坐标
 * @param irq
 * @param dev_id
 * @return
 ****************************************************/
static void StartGetPosition()
{
    TouchReg->ADCTSC = (1 << 3) | (1 << 2);
}

/****************************************************
 * 启动AD
 * @param irq
 * @param dev_id
 * @return
 **************************************************/
static void StartAD()
{
    TouchReg->ADCCON |= (1 << 0);
}

static struct timer_list TouchTimer;
static void TouchTimerFunction()
{
    if(TouchReg->ADCDAT0 & (1 << 15))
    {
        //已经松开
        WaitEnterPenDownMode();
        input_report_abs(TouchDev, ABS_PRESSURE, 0);
        input_report_key(TouchDev, BTN_TOUCH, 0);
        input_sync(TouchDev);
    }
    else
    {
        StartGetPosition();
        StartAD();
    }
}

/**************************************************
 * AD转换中断函数
 * @param irq
 * @param dev_id
 * @return
 *************************************************/
static irqreturn_t ADIRQ(int irq, void *dev_id)
{
    static int cnt = 0;
    static int x[4],y[4];

    int x_Aver,y_Aver;

    //优化2：如果ADC 转换完成后，发现release，则将此结果丢弃
    int ADC_Dat1, ADC_Dat2;
    ADC_Dat1 = TouchReg->ADCDAT0;
    ADC_Dat2 = TouchReg->ADCDAT1;

    if(TouchReg->ADCDAT0 & (1 << 15))
    {
        //已经松开
        cnt = 0;
        input_report_abs(TouchDev, ABS_PRESSURE, 0);
        input_report_key(TouchDev, BTN_TOUCH, 0);
        input_sync(TouchDev);
        WaitEnterPenDownMode();
        printk("ADC IRQ, but the touch has release\n");
    }
    else
    {
        //优化3： 多次测量求平均值
        if(cnt >= 4)
        {
            cnt = 0;
            x_Aver = (x[0] + x[1] + x[2] + x[3])/4;
            y_Aver = (y[0] + y[1] + y[2] + y[3])/4;
            //printk("Position:  x = %d, y = %d\n", x_Aver, y_Aver);

            input_report_abs(TouchDev, ABS_X, x_Aver);
            input_report_abs(TouchDev, ABS_Y, y_Aver);
            input_report_abs(TouchDev, ABS_PRESSURE, 1);
            input_report_key(TouchDev, BTN_TOUCH, 1);
            input_sync(TouchDev);

            WaitEnterPenUpMode();               //after measure, wait release

            //启动定时器，处理长按滑动
            mod_timer(&TouchTimer, jiffies + HZ/100);
        }
        else
        {
            x[cnt] = TouchReg->ADCDAT0 & 0x3FF;
            y[cnt] = TouchReg->ADCDAT1 & 0x3FF;
            //printk("x[%d] = %d, y[%d] = %d\n", cnt, x[cnt], cnt, y[cnt]);
            ++cnt;
            StartGetPosition();
            StartAD();
        }

    }

    return IRQ_HANDLED;
}


static irqreturn_t TouchIRQ(int irq, void *dev_id)
{
    if(TouchReg->ADCDAT0 & (1 << 15))
    {
        printk("Pen up\n");
        input_report_abs(TouchDev, ABS_PRESSURE, 0);
        input_report_key(TouchDev, BTN_TOUCH, 0);
        input_sync(TouchDev);
        WaitEnterPenDownMode();
    }
    else                //press
    {
        //printk("Pen down\n");
        //WaitEnterPenUpMode();
        StartGetPosition();
        StartAD();
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
    TouchReg = ioremap(0x58000000, sizeof(struct TouchRegs));
    TouchReg->ADCCON = (1 << 14) | (49 << 6);

    request_irq(IRQ_TC, TouchIRQ, IRQF_SAMPLE_RANDOM, "ts_open", NULL);
    request_irq(IRQ_ADC, ADIRQ, IRQF_SAMPLE_RANDOM, "ADC", NULL);

    //优化：电压值跳变太大，设置delay，稳定后再测量
    TouchReg->ADCDLY = 0xFFFF;

    //加入定时器，实现长按滑动的识别
    init_timer(&TouchTimer);
    TouchTimer.function = TouchTimerFunction;
    add_timer(&TouchTimer);


    TouchReg->ADCTSC = 0xd3;            //enter wait interrupt mode


    return 0;
}

static void TouchExit()
{
    free_irq(IRQ_TC, NULL);
    free_irq(IRQ_ADC, NULL);
    iounmap(TouchReg);
    input_unregister_device(TouchDev);
    input_free_device(TouchDev);
    del_timer(&TouchTimer);
}

module_init(TouchInit);
module_exit(TouchExit);

MODULE_LICENSE("GPL");