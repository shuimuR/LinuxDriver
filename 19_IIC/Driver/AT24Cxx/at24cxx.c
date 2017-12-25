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
#include <linux/fs.h>
#include <asm/uaccess.h>


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
struct i2c_client *AT24Cxx_client;

static ssize_t AT24Cxx_Read(struct file *file, char __user *buf, size_t size, loff_t * offset)
{
    unsigned char address;
    unsigned char data;
    struct i2c_msg msg[2];
    int ret;

    if(size != 1)
        return -EINVAL;

    copy_from_user(&address, buf, 1);

    //写数据地址
    msg[0].addr = AT24Cxx_client->addr;
    msg[0].buf = &address;
    msg[0].len = 1;
    msg[0].flags = 0;

    //读数据
    msg[1].addr = AT24Cxx_client->addr;
    msg[1].buf = &data;
    msg[1].len = 1;
    msg[1].flags = I2C_M_RD;

    ret = i2c_transfer(AT24Cxx_client->adapter, msg, 2);
    if(ret == 2)
    {
        copy_to_user(&buf[0], &data, 1);
        return 1;
    }
    else
    {
        return -EIO;
    }
}

static ssize_t AT24Cxx_Write(struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
    unsigned char val[2];
    struct i2c_msg msg[1];
    int ret;

    if(size != 2)
        return -EINVAL;
    copy_from_user(val, buf, 2);

    msg[0].addr = AT24Cxx_client->addr;
    msg[0].buf = val;
    msg[0].len = 2;
    msg[0].flags = 0;

    ret = i2c_transfer(AT24Cxx_client->adapter, msg, 1);
    if(ret == 1)
        return 2;
    else
        return -EIO;
}

static int major;
static struct file_operations AT24Cxx_Fops =
        {
                .owner = THIS_MODULE,
                .read = AT24Cxx_Read,
                .write = AT24Cxx_Write,
        };

static struct class *AT24CxxClass;
static struct class_device *AT24CxxDev;

static int AT24CXX_detect(struct i2c_adapter *adapter, int address, int kind)
{
    printk("AT24CXX_detect\n");

    AT24Cxx_client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL);

    AT24Cxx_client->addr = address;
    AT24Cxx_client->adapter = adapter;
    AT24Cxx_client->driver = &AT24Cxx_Driver;
    strcpy(AT24Cxx_client->name, "AT24Cxx");

    i2c_attach_client(AT24Cxx_client);

    major = register_chrdev(0, "AT24Cxx", &AT24Cxx_Fops);

    AT24CxxClass = class_create(THIS_MODULE, "AT24Cxx");
    if(IS_ERR(AT24CxxClass))
        return PTR_ERR(AT24CxxClass);

    AT24CxxDev = class_device_create(AT24CxxClass, NULL, MKDEV(major, 0), NULL, "AT24Cxx");			//创建类FirstDrv下的/dev/xyz节点
    if(unlikely(IS_ERR(AT24CxxDev)))
        return PTR_ERR(AT24CxxDev);

    return 0;
}


static int AT24CXX_Attach(struct i2c_adapter *adapter)
{
    return i2c_probe(adapter, &addr_data, AT24CXX_detect);
}

static int AT24CXX_Detach(struct i2c_client *client)
{
    printk("AT24CXX_Detach\n");
    unregister_chrdev(major, "AT24Cxx");
    class_device_unregister(AT24CxxDev);
    class_destroy(AT24CxxClass);

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