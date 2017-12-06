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

#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>

struct pin_desc
{
	int irq;
	char *name;
	unsigned int pin;
	unsigned int key_val;
};

struct pin_desc pindescs[4] = 
{
	{IRQ_EINT0, "S2", S3C2410_GPF0, KEY_L},
	{IRQ_EINT2, "S3", S3C2410_GPF2, KEY_S},
	{IRQ_EINT11, "S4", S3C2410_GPG3, KEY_ENTER},
	{IRQ_EINT19, "S5", S3C2410_GPG11, KEY_LEFTSHIFT},
};

/**************************************************************
 * 中断处理函数
 *************************************************************/
static struct timer_list ButtonTimer;
static struct pin_desc *irq_pd = NULL;
static struct input_dev *ButtonDev;
static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	irq_pd = (struct pin_desc *)dev_id;
	mod_timer(&ButtonTimer, jiffies + HZ/100);		//jiffies every 10 ms system timeout
	return IRQ_HANDLED;
}

static void ButtonTimeOut(unsigned long time)
{
	struct pin_desc *CurrentDev= irq_pd;
	unsigned int pinval;

	if(!CurrentDev)
		return;

	pinval = s3c2410_gpio_getpin(CurrentDev->pin);

	//input_event最后一个参数 0---松开  1----按下
	if(pinval)
	{
		input_event(ButtonDev, EV_KEY, CurrentDev->key_val, 0);
		input_sync(ButtonDev);
	}
	else
	{
		input_event(ButtonDev, EV_KEY, CurrentDev->key_val, 1);
		input_sync(ButtonDev);
	}
}

static int ButtonsInit(void)
{
	int i;
	int ret;
	//1. 分配一个input_dev结构体
	ButtonDev = input_allocate_device();
	//2. 设置
	set_bit(EV_KEY, ButtonDev->evbit);			//事件类型
	set_bit(EV_REP, ButtonDev->evbit);			//产生重复按键

	//产生事件  L， S, Enter, Leftshift
	set_bit(KEY_L, ButtonDev->keybit);
	set_bit(KEY_S, ButtonDev->keybit);
	set_bit(KEY_ENTER, ButtonDev->keybit);
	set_bit(KEY_LEFTSHIFT, ButtonDev->keybit);

	//3. 注册
	ret = input_register_device(ButtonDev);

	//4. 硬件相关的操作
	for(i = 0; i < 4; i++)
	{
		request_irq(pindescs[i].irq, buttons_irq, IRQT_BOTHEDGE, pindescs[i].name, &pindescs[i]);
	}

	init_timer(&ButtonTimer);
	ButtonTimer.function = ButtonTimeOut;
	add_timer(&ButtonTimer);

	return 0;
}

static void ButtonsExit(void)
{
	int i;
	for(i = 0; i < 4; i++)
	{
		free_irq(pindescs[i].irq, &pindescs[i]);
	}
	del_timer(&ButtonTimer);
	input_unregister_device(ButtonDev);
	input_free_device(ButtonDev);
}

module_init(ButtonsInit);
module_exit(ButtonsExit);

MODULE_LICENSE("GPL");

