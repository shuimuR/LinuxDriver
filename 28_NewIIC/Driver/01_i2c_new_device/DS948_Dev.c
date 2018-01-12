#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/slab.h>

static struct i2c_board_info DS948_info = {
   I2C_BOARD_INFO("DS948", 0x2C),
};

static struct i2c_client *DS948_Client;

static int DS948_Dev_Init(void)
{
    struct i2c_adapter *i2c_adapter;
    printk("i2c_get_adapter\n");
    i2c_adapter = i2c_get_adapter(0);
    printk("i2c_new_device\n");
    DS948_Client = i2c_new_device(i2c_adapter, &DS948_info);
    printk("i2c_put_adapter\n");
    i2c_put_adapter(i2c_adapter);
    return 0;
}

static void DS948_Dev_Exit(void)
{
    i2c_unregister_device(DS948_Client);
}

module_init(DS948_Dev_Init);
module_exit(DS948_Dev_Exit);

MODULE_LICENSE("GPL");
