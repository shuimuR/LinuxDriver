/********************************************
 * 参考文件
 * drivers/block/xd.c
 * drivers/block/z2ram.c
 */

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/blkdev.h>
#include <linux/blkpg.h>
#include <linux/delay.h>
#include <linux/io.h>

#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/dma.h>

static struct gendisk *ramblock_disk;
static request_queue_t *ramblock_queue;

static DEFINE_SPINLOCK(ramblock_lock);

static int major;

static struct block_device_operations ramblock_fops = {
        .owner	= THIS_MODULE,
};

#define RamBlockSize (1024*1024)

static unsigned char *ram_BlockBuffer;

static void do_ramblock_request(request_queue_t * q)
{
    static int cnt = 0;
    struct request *req;
    printk("do_ramblock_request %d\n", ++cnt);

    while ((req = elv_next_request(q)) != NULL)
    {
        //数据传输三要素： 源  目的  长度
        unsigned long offset = req->sector * 512;

        unsigned long len = req->current_nr_sectors * 512;

        if(rq_data_dir(req) == READ)
        {
            memcpy(req->buffer, ram_BlockBuffer + offset, len);
        }
        else
        {
            memcpy(ram_BlockBuffer + offset, req->buffer, len);
        }

        end_request(req, 1);	/* wrap up, 0 = fail, 1 = success */
    }
}

static int BlockInit(void)
{
    //1. 分配一个gendisk结构体
    ramblock_disk = alloc_disk(2);       //次设备号的个数：最多多少分区

    //2.设置
    //2.1 分配一个队列，提供读写能力
    ramblock_queue = blk_init_queue(do_ramblock_request, &ramblock_lock);
    ramblock_disk->queue = ramblock_queue;
    //2.2 设置其他属性
    major = register_blkdev(0, "ramblock");

    ramblock_disk->major = major;
    ramblock_disk->first_minor = 0;
	sprintf(ramblock_disk->disk_name, "ramblock");
    ramblock_disk->fops = &ramblock_fops;

    set_capacity(ramblock_disk, RamBlockSize/512);

    ram_BlockBuffer = kzalloc(RamBlockSize, GFP_KERNEL);

    //3. 注册
    add_disk(ramblock_disk);

    return 0;
}

static void BlockExit(void)
{
    unregister_blkdev(major, "ramblock");
    del_gendisk(ramblock_disk);
    put_disk(ramblock_disk);

    blk_cleanup_queue(ramblock_queue);
    kfree(ram_BlockBuffer);
}

module_init(BlockInit);
module_exit(BlockExit);

MODULE_LICENSE("GPL");
