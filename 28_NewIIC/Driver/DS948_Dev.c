#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/slab.h>

static struct i2c_client *DS948_Client;

static const unsigned short normal_i2c[] = { 0x2c, 0x2d, I2C_CLIENT_END };

static int DS948_Dev_Init(void)
{
	struct i2c_board_info i2c_info;

	memset(&i2c_info, 0, sizeof(struct i2c_board_info));
	strlcpy(i2c_info.type, "DS948", I2C_NAME_SIZE);

    struct i2c_adapter *i2c_adapter;
    i2c_adapter = i2c_get_adapter(0);
    DS948_Client = i2c_new_probed_device(i2c_adapter, &i2c_info,
						   normal_i2c, NULL);
    i2c_put_adapter(i2c_adapter);

    if(DS948_Client)
        return 0;
    else
    {
        printk("there's no i2c device\n");
        return -1;
    }
}

static void DS948_Dev_Exit(void)
{
    i2c_unregister_device(DS948_Client);
}

module_init(DS948_Dev_Init);
module_exit(DS948_Dev_Exit);

MODULE_LICENSE("GPL");
