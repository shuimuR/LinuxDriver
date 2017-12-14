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

static int USB_Mouse_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
    printk("USB mouse insert \n");

    struct usb_device *dev = interface_to_usbdev(intf);

    printk("bcdUSB = %x\n", dev->descriptor.bcdUSB);
    printk("VID = %x\n", dev->descriptor.idVendor);
    printk("PID = %x\n", dev->descriptor.idProduct);
    return 0;
}

static void USB_Mouse_disconnect(struct usb_interface *intf)
{
    printk("USB mouse disconnect\n");
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

