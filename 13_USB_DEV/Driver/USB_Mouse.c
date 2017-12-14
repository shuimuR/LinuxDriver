#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>

static struct usb_device_id USB_Mouse_id_table [] = {
        { USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
                             USB_INTERFACE_PROTOCOL_MOUSE) },
        { }	/* Terminating entry */
};

static struct input_dev *uk_dev;
static char *usb_buf;
static dma_addr_t *usb_buf_phy;
int len;
static struct urb *uk_urb;

static void USB_MOUSE_AS_Key_IRQ(struct urb *urb)
{
    int i;
    static int cnt = 0;
    /*printk("data cnt = %d\n", ++cnt);
    for(i = 0; i < len; i++)
    {
        printk("%02x ", usb_buf[i]);
    }*/

    //解析USB 数据含义并上报
    static unsigned char pre_val = 0;
    if((pre_val & (1 << 0)) != (usb_buf[0] & (1 << 0)))
    {
        //左键发生变化
        input_event(uk_dev, EV_KEY, KEY_L, (usb_buf[0] & (1 << 0)) ? 1 : 0);
        input_sync(uk_dev);
    }

    if((pre_val & (1 << 1)) != (usb_buf[0] & (1 << 1)))
    {
        //右键发生变化
        input_event(uk_dev, EV_KEY, KEY_S, (usb_buf[0] & (1 << 1)) ? 1 : 0);
        input_sync(uk_dev);
    }

    if((pre_val & (1 << 2)) != (usb_buf[0] & (1 << 2)))
    {
        //中键发生变化
        input_event(uk_dev, EV_KEY, KEY_ENTER, (usb_buf[0] & (1 << 2)) ? 1 : 0);
        input_sync(uk_dev);
    }
    pre_val = usb_buf[0];
    //重新提交urb
    usb_submit_urb(uk_urb, GFP_KERNEL);
}

static int USB_Mouse_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
    printk("USB mouse insert \n");

    struct usb_device *dev = interface_to_usbdev(intf);
    struct usb_host_interface *interface;
    struct usb_endpoint_descriptor *endpoint;
    int pipe;

    interface = intf->cur_altsetting;

    if (interface->desc.bNumEndpoints != 1)
        return -ENODEV;

    endpoint = &interface->endpoint[0].desc;
    if (!usb_endpoint_is_int_in(endpoint))
        return -ENODEV;

    printk("bcdUSB = %x\n", dev->descriptor.bcdUSB);
    printk("VID = %x\n", dev->descriptor.idVendor);
    printk("PID = %x\n", dev->descriptor.idProduct);

    //1. 分配一个input_dev
    uk_dev = input_allocate_device();
    //2.设置
    //2.1能产生哪类事件
    set_bit(EV_KEY, uk_dev->evbit);
    set_bit(EV_REP, uk_dev->evbit);

    //2.2 能产生哪些事件
    set_bit(KEY_L, uk_dev->keybit);
    set_bit(KEY_S, uk_dev->keybit);
    set_bit(KEY_ENTER, uk_dev->keybit);

    //3. 注册
    input_register_device(uk_dev);

    //4. 硬件相关操作
    //4.1 源：USB设备的某个端点
    pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);

    //4.2 长度
    len = endpoint->wMaxPacketSize;
    //4.3 目的： 将读取到的数据保存在缓冲区
    usb_buf = usb_buffer_alloc(dev, len, GFP_ATOMIC, &usb_buf_phy);

    //使用 usb request block
    uk_urb =  usb_alloc_urb(0, GFP_KERNEL);

    usb_fill_int_urb(uk_urb, dev, pipe, usb_buf, len, USB_MOUSE_AS_Key_IRQ, NULL, endpoint->bInterval);
    uk_urb->transfer_dma = usb_buf_phy;
    uk_urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;


    //使用urb
    usb_submit_urb(uk_urb, GFP_KERNEL);
    return 0;
}

static void USB_Mouse_disconnect(struct usb_interface *intf)
{
    printk("USB mouse disconnect\n");
    struct usb_device *dev = interface_to_usbdev(intf);
    usb_kill_urb(uk_urb);
    usb_free_urb(uk_urb);
    usb_buffer_free(dev, len, usb_buf, usb_buf_phy);
    input_unregister_device(uk_dev);
    input_free_device(uk_dev);
}

static struct usb_driver USB_MouseAsKeyDriver = {
        .name		= "USBMouseAsKey",
        .probe		= USB_Mouse_probe,
        .disconnect	= USB_Mouse_disconnect,
        .id_table	= USB_Mouse_id_table,
};

static int USB_MouseAsKeyInit()
{
    int retval = usb_register(&USB_MouseAsKeyDriver);

    return 0;
}

static void USB_MouseAsKeyExit()
{
    usb_deregister(&USB_MouseAsKeyDriver);
}

module_init(USB_MouseAsKeyInit);
module_exit(USB_MouseAsKeyExit);

MODULE_LICENSE("GPL");

