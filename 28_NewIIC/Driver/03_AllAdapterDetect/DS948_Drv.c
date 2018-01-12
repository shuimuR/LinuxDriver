#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/poll.h>
#include <linux/cdev.h>

static const struct i2c_device_id DS948_id_table[] = {
	{ "DS948", 0 },
	{}
};

static const unsigned short addr_list[] = { 0x60, 0x2C, I2C_CLIENT_END };

static int DS948_Probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
    printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    return 0;
}

static int DS948_Remove(struct i2c_client *client)
{
    printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}

static int DS948_Detect(struct i2c_client *client,
		       struct i2c_board_info *info)
{
	/* 能运行到这里, 表示该addr的设备是存在的
	 * 但是有些设备单凭地址无法分辨(A芯片的地址是0x50, B芯片的地址也是0x50)
	 * 还需要进一步读写I2C设备来分辨是哪款芯片
	 * detect就是用来进一步分辨这个芯片是哪一款，并且设置info->type
	 */

	printk("DS948_Detect : addr = 0x%x\n", client->addr);

	/* 进一步判断是哪一款 */

	strlcpy(info->type, "DS948", I2C_NAME_SIZE);
	return 0;
}

static struct i2c_driver DS948_Drv =
{
	.class  = I2C_CLASS_HWMON, /* 表示去哪些适配器上找设备 */
    .driver =
    {
        .name = "DS948",
        .owner = THIS_MODULE,
    },
    .probe = DS948_Probe,
    .remove = DS948_Remove,
    .id_table = DS948_id_table,
	.detect     = DS948_Detect,  /* 用这个函数来检测设备确实存在 */
	.address_list	= addr_list,   /* 这些设备的地址 */
};

static int DS948_Drv_Init(void)
{
    int ret;
    printk("Start  i2c_add_driver\n");
	ret = i2c_add_driver(&DS948_Drv);
	if (ret != 0)
		pr_err("Failed to register DS948 I2C driver: %d\n", ret);
    printk("i2c_add_driver finish\n");
	return ret;
}

static void DS948_Drv_Exit(void)
{
    i2c_del_driver(&DS948_Drv);
}

module_init(DS948_Drv_Init);
module_exit(DS948_Drv_Exit);

MODULE_LICENSE("GPL");
