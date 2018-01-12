#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>


struct proc_dir_entry *entry;

#define BufferLen 1024
static char mylog_buf[BufferLen];
static char temp_buf[BufferLen];
static int MyLogW = 0;
static int MyLogR = 0;
static int MyLogR_ForRead = 0;

static DECLARE_WAIT_QUEUE_HEAD(log_wait);

static int IsMyLogEmpty(void)
{
    return (MyLogW == MyLogR);
}

static int IsMyLogEmptyForRead(void)
{
    return (MyLogW == MyLogR_ForRead);
}

static int IsMyLogFull(void)
{
    if((MyLogW + 1) % BufferLen == MyLogR)
        return 1;
    else
        return 0;
}

static int IsMyLogFullForRead(void)
{
    if((MyLogW + 1) % BufferLen == MyLogR_ForRead)
        return 1;
    else
        return 0;
}

static void MyLogPutc(char c)
{
    if(IsMyLogFull())
    {
        //丢弃一个老数据
        MyLogR = (MyLogR + 1) % BufferLen;
        if((MyLogR_ForRead + 1) % BufferLen == MyLogR)
            MyLogR_ForRead = MyLogR;
    }
    mylog_buf[MyLogW] = c;
    MyLogW = (MyLogW + 1) % BufferLen;

    wake_up_interruptible(&log_wait);
}

static int MyLogGetc(char *p)
{
    if(IsMyLogEmpty())
        return 0;
    else
    {
        *p = mylog_buf[MyLogR];
        MyLogR = (MyLogR + 1) % BufferLen;
    }
	return 1;
}

static int MyLogGetcForRead(char *p)
{
    if(IsMyLogEmptyForRead())
        return 0;
    else
    {
        *p = mylog_buf[MyLogR_ForRead];
        MyLogR_ForRead = (MyLogR_ForRead + 1) % BufferLen;
    }
	return 1;
}

int MyPrintk(const char *fmt, ...)
{
    va_list args;
    int i, j;

    va_start(args, fmt);
    i = vsnprintf(temp_buf, INT_MAX, fmt, args);
    va_end(args);

    for(j = 0; j < i; j++)
    {
        MyLogPutc(temp_buf[j]);
    }

	return i;
}

static int MyMsgOpen(struct inode *inode, struct file *file)
{
    MyLogR_ForRead = MyLogR;
    return 0;
}

static ssize_t MyMsgRead(struct file *file, char __user *buf,
                        size_t count, loff_t *ppos)
{
    int error = 0;
    char c;
    int i = 0;

    if((file->f_flags & O_NONBLOCK) && IsMyLogEmptyForRead())
        return -EAGAIN;

    error = wait_event_interruptible(log_wait, !IsMyLogEmptyForRead());

    //copy to __user
    while(!error && (MyLogGetcForRead(&c)) && i < count)
    {
        error = __put_user(c, buf);
        buf++;
        i++;
    }

    if(!error)
        error = i;

    return error;
}

const struct file_operations MyMsgOps =
{
    .open = MyMsgOpen,
    .read = MyMsgRead,
};

static int ProcPrintkInit(void)
{
    entry = create_proc_entry("MyMsg", S_IRUSR, &proc_root);
    if (entry)
        entry->proc_fops = &MyMsgOps;
    return 0;
}

static void ProcPrintkExit(void)
{
    remove_proc_entry("MyMsg", &proc_root);
}

module_init(ProcPrintkInit);
module_exit(ProcPrintkExit);

EXPORT_SYMBOL(MyPrintk);

MODULE_LICENSE("GPL");
