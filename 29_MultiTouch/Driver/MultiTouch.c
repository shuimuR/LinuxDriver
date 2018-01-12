
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <asm/mach/irq.h>

#include <linux/gpio.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>

#define MTP_Addr (0x70>>1)

#define MTP_MAX_X	800
#define MTP_MAX_Y	480
#define MTP_MAX_ID 15 /* 由硬件决定 */
#define MTP_NAME "ft5x0x_ts"

#define MTP_IRQ  gpio_to_irq(EXYNOS4_GPX1(6))

struct input_dev *MTP_Dev;
static struct work_struct mtp_work;
static struct i2c_client *MTP_Client;

struct mtp_event {
	int x;
	int y;
	int id;
};
static struct mtp_event mtp_events[16];
static int mtp_points;

//static irqreturn_t MTP_Interrupt(int irq, void *dev_id)
static irqreturn_t mtp_interrupt(int irq, void *dev_id)
{
	/* 本该:
	 * 获取触点数据, 并上报
	 * 但是I2C是慢速设备, 不该放在中断服务程序中操作
	 */

	/* 使用工作队列, 让内核线程来操作 */
	printk("MTP interrupt, schedule_work\n");
	schedule_work(&mtp_work);

	return IRQ_HANDLED;
}

static int mtp_ft5x0x_i2c_rxdata(struct i2c_client *client, char *rxdata, int length) {
	int ret;
	struct i2c_msg msgs[] = {
		{
			.addr	= client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= rxdata,
		},
		{
			.addr	= client->addr,
			.flags	= I2C_M_RD,
			.len	= length,
			.buf	= rxdata,
		},
	};
	printk("mtp_ft5x0x_i2c_rxdata\n");
	ret = i2c_transfer(client->adapter, msgs, 2);
	if (ret < 0)
	{
		printk("mtp_ft5x0x_i2c_rxdata i2c_transfer error\n");
		pr_err("%s: i2c read error: %d\n", __func__, ret);
	}

	return ret;
}

static int mtp_ft5x0x_read_data(void) {
	u8 buf[32] = { 0 };
	int ret;
	printk("%s start call mtp_ft5x0x_i2c_rxdata\n", __FUNCTION__);
	ret = mtp_ft5x0x_i2c_rxdata(MTP_Client, buf, 31);

	if (ret < 0) {
		printk("%s read data error\n", __FUNCTION__);
		printk("%s: read touch data failed, %d\n", __func__, ret);
		return ret;
	}

	mtp_points = buf[2] & 0x0f;

	printk("%s start analysis data\n", __FUNCTION__);
	switch (mtp_points) {
		case 5:
			mtp_events[4].x = (s16)(buf[0x1b] & 0x0F)<<8 | (s16)buf[0x1c];
			mtp_events[4].y = (s16)(buf[0x1d] & 0x0F)<<8 | (s16)buf[0x1e];
			mtp_events[4].id = buf[0x1d]>>4;
		case 4:
			mtp_events[3].x = (s16)(buf[0x15] & 0x0F)<<8 | (s16)buf[0x16];
			mtp_events[3].y = (s16)(buf[0x17] & 0x0F)<<8 | (s16)buf[0x18];
			mtp_events[3].id = buf[0x17]>>4;
		case 3:
			mtp_events[2].x = (s16)(buf[0x0f] & 0x0F)<<8 | (s16)buf[0x10];
			mtp_events[2].y = (s16)(buf[0x11] & 0x0F)<<8 | (s16)buf[0x12];
			mtp_events[2].id = buf[0x11]>>4;
		case 2:
			mtp_events[1].x = (s16)(buf[0x09] & 0x0F)<<8 | (s16)buf[0x0a];
			mtp_events[1].y = (s16)(buf[0x0b] & 0x0F)<<8 | (s16)buf[0x0c];
			mtp_events[1].id = buf[0x0b]>>4;
		case 1:
			mtp_events[0].x = (s16)(buf[0x03] & 0x0F)<<8 | (s16)buf[0x04];
			mtp_events[0].y = (s16)(buf[0x05] & 0x0F)<<8 | (s16)buf[0x06];
			mtp_events[0].id = buf[0x05]>>4;
			break;
		case 0:
			return 0;
		default:
			//printk("%s: invalid touch data, %d\n", __func__, event->touch_point);
			return -1;
	}

	return 0;
}

static void MTP_WorkFunc(struct work_struct *work)
{
	int i;
	int ret;

	/* 读取I2C设备, 获得触点数据, 并上报 */
	/* 读取 */
	printk("%s mtp_ft5x0x_read_data\n", __FUNCTION__);
	ret = mtp_ft5x0x_read_data();
	if (ret < 0)
	{
		printk("%s read data error\n", __FUNCTION__);
		return;
	}

	/* 上报 */
	//触摸完成后上报
	if (!mtp_points) {
		printk("%s finish touch operation\n", __FUNCTION__);
		input_mt_sync(MTP_Dev);
		input_sync(MTP_Dev);
		return;
	}
	printk("Start to report the MTP points\n");
	for (i = 0; i < mtp_points; i++) { /* 每一个点 */
	    input_report_abs(MTP_Dev, ABS_MT_POSITION_X, mtp_events[i].x);
	    input_report_abs(MTP_Dev, ABS_MT_POSITION_Y, mtp_events[i].y);
	    input_report_abs(MTP_Dev, ABS_MT_TRACKING_ID, mtp_events[i].id);
	    input_mt_sync(MTP_Dev);
	}
	input_sync(MTP_Dev);
}

static int __devinit MultiTouch_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
	//在对比名称时，如果有相同名称的dev设备时，调用此函数
	printk("**************************************%s %s %d**************************************************\n", __FILE__, __FUNCTION__, __LINE__);

	MTP_Client = client;

	printk("%s input_allocate_device\n", __FUNCTION__);
	//1. 分配一个input_dev结构体
	MTP_Dev = input_allocate_device();

	//2. 设置
	//2.1 能产生哪类事件
	printk("%s set_bit\n", __FUNCTION__);
	set_bit(EV_SYN, MTP_Dev->evbit);
	set_bit(EV_ABS, MTP_Dev->evbit);

	set_bit(INPUT_PROP_DIRECT, MTP_Dev->propbit);
	//2.2能产生这类事件中的哪些
	set_bit(ABS_MT_TRACKING_ID, MTP_Dev->absbit);
	set_bit(ABS_MT_POSITION_X, MTP_Dev->absbit);
	set_bit(ABS_MT_POSITION_Y, MTP_Dev->absbit);

	//2.3这类事件的范围
	printk("%s input_set_abs_params\n", __FUNCTION__);
	input_set_abs_params(MTP_Dev, ABS_MT_TRACKING_ID, 0, MTP_MAX_ID, 0, 0);
	input_set_abs_params(MTP_Dev, ABS_MT_POSITION_X, 0, MTP_MAX_X, 0, 0);
	input_set_abs_params(MTP_Dev, ABS_MT_POSITION_Y, 0, MTP_MAX_Y, 0, 0);

	printk("%s set MTP_Dev name\n", __FUNCTION__);
	MTP_Dev->name = MTP_NAME;

	//硬件相关操作
	input_register_device(MTP_Dev);

	printk("%s INIT_WORK\n", __FUNCTION__);
	INIT_WORK(&mtp_work, MTP_WorkFunc);

	//触摸屏芯片有数据时触发中断
	printk("%s request_irq\n", __FUNCTION__);
	//request_irq(MTP_IRQ, MTP_Interrupt, IRQ_TYPE_EDGE_FALLING, "100ask_mtp", MTP_Dev);
	request_irq(MTP_IRQ, mtp_interrupt,
			IRQ_TYPE_EDGE_FALLING /*IRQF_TRIGGER_FALLING*/, "100ask_mtp", MTP_Dev);

	return 0;
}

static int __devexit MultiTouch_remove(struct i2c_client *client)
{
	printk("******************************************%s %s %d***********************************************\n", __FILE__, __FUNCTION__, __LINE__);

	printk("%s free_irq\n", __FUNCTION__);
	free_irq(MTP_IRQ, MTP_Dev);
	printk("%s cancel_work_sync\n", __FUNCTION__);
	cancel_work_sync(&mtp_work);

	printk("%s input_unregister_device\n", __FUNCTION__);
	input_unregister_device(MTP_Dev);
	input_free_device(MTP_Dev);

	return 0;
}

static const struct i2c_device_id MultiTouch_id_table[] = {
	{ "100ask_mtp", 0 },  /* 支持我们自己的mtp_driver自行探测到的I2C设备 */
	{ "ft5x0x_ts", 0},    /* 支持mach-tiny4412.c中注册的名为"ft5x0x_ts"的I2C设备 */
	{}
};

static int mtp_ft5x06_valid(struct i2c_client *client)
{
	u8 buf[32] = { 0 };
	int ret;

	printk("mtp_ft5x06_valid : addr = 0x%x\n", client->addr);

	/* 进一步判断设备的合法性 */
	printk("%s mtp_ft5x0x_i2c_rxdata\n", __FUNCTION__);
	buf[0] = 0xa3; /* chip vendor id */
	ret = mtp_ft5x0x_i2c_rxdata(client, buf, 1);

	if (ret < 0) {
		printk("There is not real device, i2c read err\n");
		return ret;
	}
	printk("chip vendor id = 0x%x\n", buf[0]);

	if (buf[0] != 0x55){
		printk("There is not real device, val err\n");
		return -1;
	}
	return 0;
}

static int MultiTouch_detect(struct i2c_client *client,
		       struct i2c_board_info *info)
{
	//在遍历地址时，如果发现设备，则调用此函数进一步识别
	printk("MultiTouch_detect : addr = 0x%x\n", client->addr);

	if (mtp_ft5x06_valid(client) < 0)
	{
		printk("%s mtp_ft5x06_valid error\n", __FUNCTION__);
		return -1;
	}

	strlcpy(info->type, "100ask_mtp", I2C_NAME_SIZE);
	return 0;

	//返回0之后，会创建一个新的IIC设备
}

static const unsigned short addr_list[] = { MTP_Addr, I2C_CLIENT_END };

static struct i2c_driver MultiTouch_driver = {
	.class  = I2C_CLASS_HWMON,
	.driver	= {
		.name	= "100ask",
		.owner	= THIS_MODULE,
	},
	.probe		= MultiTouch_probe,
	.remove		= __devexit_p(MultiTouch_remove),
	.id_table	= MultiTouch_id_table,
	.detect     = MultiTouch_detect,
	.address_list	= addr_list,
};

static int MultiTouch_drv_init(void)
{
	i2c_add_driver(&MultiTouch_driver);

	return 0;
}

static void MultiTouch_drv_exit(void)
{
	i2c_del_driver(&MultiTouch_driver);
}


module_init(MultiTouch_drv_init);
module_exit(MultiTouch_drv_exit);
MODULE_LICENSE("GPL");
