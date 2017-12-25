/************************
 * 参考内核drivers/i2c/chips/eeprom.c
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/mutex.h>

static unsigned short ignore[] = { I2C_CLIENT_END };
//地址值为7位
static unsigned short normal_addr[] = { 0x50, I2C_CLIENT_END };

static unsigned short ForceAddr[3] = {ANY_I2C_BUS, 0x60, I2C_CLIENT_END};
static unsigned short *Forces[] = {ForceAddr, NULL};

static struct i2c_client_address_data addr_data = {
        .normal_i2c	= normal_addr,
        .probe		= ignore,
        .ignore		= ignore,
        //.forces     = Forces,           /*强制认为存在这个设备*/
};

static struct i2c_driver AT24Cxx_Driver;

static int AT24CXX_detect(struct i2c_adapter *adapter, int address, int kind)
{
    printk("AT24CXX_detect\n");
    struct i2c_client *new_client;
    new_client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL);

    new_client->addr = address;
    new_client->adapter = adapter;
    new_client->driver = &AT24Cxx_Driver;
    strcpy(new_client->name, "AT24Cxx");

    i2c_attach_client(new_client);

    return 0;
}


static int AT24CXX_Attach(struct i2c_adapter *adapter)
{
    return i2c_probe(adapter, &addr_data, AT24CXX_detect);
}

static int AT24CXX_Detach(struct i2c_client *client)
{
    printk("AT24CXX_Detach\n");
    i2c_detach_client(client);
    kfree(i2c_get_clientdata(client));
    return 0;
}

//1. 分配一个i2c_driver结构体
static struct i2c_driver AT24CXX_Driver =
        {
                .driver = {
                                .name = "AT24CXX",
                        },
                .attach_adapter = AT24CXX_Attach,
                .detach_client = AT24CXX_Detach,
        };



static int AT24Cxx_Init(void)
{
    i2c_add_driver(&AT24CXX_Driver);
    return 0;
}

static void AT24Cxx_Exit(void)
{
    i2c_del_driver(&AT24CXX_Driver);
}

module_init(AT24Cxx_Init);
module_exit(AT24Cxx_Exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shuimu");
MODULE_DESCRIPTION("I2C AT24CXX driver");