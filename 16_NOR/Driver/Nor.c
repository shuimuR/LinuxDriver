/************************************************
参考drivers/mtd/maps/physmap.c
*************************************************/
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/physmap.h>
#include <asm/io.h>

struct map_info	*s3c_nor = NULL;
struct mtd_info *s3c_mtd = NULL;

static struct mtd_partition s3c_nor_parts[] = {
        [0] = {
                .name   = "bootloader_nor",
                .size   = 0x00040000,
                .offset	= 0,
        },
        [1] = {
                .name   = "root",
                .offset = MTDPART_OFS_APPEND,
                .size   = MTDPART_SIZ_FULL,
        }
};

static int NorInit(void)
{
    //1. 分配map_info结构体
    s3c_nor = kzalloc(sizeof(struct map_info), GFP_KERNEL);

    //2. 设置物理基地址，大小，位宽，虚拟基地址
    s3c_nor->name = "s3c_nor";
    s3c_nor->phys = 0;
    s3c_nor->size = 0x1000000;
    s3c_nor->bankwidth = 2;

    s3c_nor->virt = ioremap(s3c_nor->phys, s3c_nor->size);

    simple_map_init(s3c_nor);

    //3. 使用
    printk("use cfi_probe\n");
    s3c_mtd = do_map_probe("cfi_probe", s3c_nor);
    if(!s3c_mtd)
    {
        printk("use jedec_probe\n");
        s3c_mtd = do_map_probe("jedec_probe", s3c_nor);
        if(!s3c_mtd)
        {
            iounmap(s3c_nor->virt);
            kfree(s3c_nor);
            printk("do_map_probe fail\n");
            return -EIO;
        }
    }

    //4.分区
    add_mtd_partitions(s3c_mtd, s3c_nor_parts, 2);

    return 0;
}

static void NorExit(void)
{
    del_mtd_partitions(s3c_mtd);
    iounmap(s3c_nor->virt);
    kfree(s3c_nor);
}

module_init(NorInit);
module_exit(NorExit);

MODULE_LICENSE("GPL");