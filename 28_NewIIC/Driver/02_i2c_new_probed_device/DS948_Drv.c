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

static int major;

static ssize_t DS90UB948_Read(struct file *file, char __user *buf, size_t size, loff_t * offset);
static ssize_t DS90UB948_Write(struct file *file, const char __user *buf, size_t size, loff_t *offset);

static struct file_operations DS90UB948_Fops =
{
	.owner = THIS_MODULE,
	.read = DS90UB948_Read,
	.write = DS90UB948_Write,
};

static struct class *DS90UB948_Class;

static struct i2c_client *DS948_client;

static const struct i2c_device_id DS948_id_table[] = {
	{ "DS948", 0 },
	{}
};

static ssize_t DS90UB948_Read(struct file *file, char __user *buf, size_t size, loff_t * offset)
{
	unsigned char addr, data;

	copy_from_user(&addr, buf, 1);
	data = i2c_smbus_read_byte_data(DS948_client, addr);
	copy_to_user(buf, &data, 1);
	return 1;
}

/****************************************************************
 * 948 的初始化采用每次两个字节的写入，Addr_Data, Data
 * @param file
 * @param __user
 * @return
 */
static ssize_t DS90UB948_Write(struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
	unsigned char ker_buf[2];
	unsigned char addr, data;

	copy_from_user(ker_buf, buf, 2);
	addr = ker_buf[0];
	data = ker_buf[1];

	printk("addr = 0x%02x, data = 0x%02x\n", addr, data);

	if (!i2c_smbus_write_byte_data(DS948_client, addr, data))
		return 2;
	else
		return -EIO;
}

static int DS948_Probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
    printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

	DS948_client = client;

	major = register_chrdev(0, "DS90UB948", &DS90UB948_Fops);

	DS90UB948_Class = class_create(THIS_MODULE, "DS90UB948");
	device_create(DS90UB948_Class, NULL, MKDEV(major, 0), NULL, "DS90UB948"); /* /dev/at24cxx */

    return 0;
}

static int DS948_Remove(struct i2c_client *client)
{
    printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	unregister_chrdev(major, "DS90UB948");
	device_destroy(DS90UB948_Class, MKDEV(major, 0));
	class_destroy(DS90UB948_Class);
	return 0;
}

static struct i2c_driver DS948_Drv =
{
    .driver =
    {
        .name = "DS948",
        .owner = THIS_MODULE,
    },
    .probe = DS948_Probe,
    .remove = DS948_Remove,
    .id_table = DS948_id_table,
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
