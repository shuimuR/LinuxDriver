/**************************************************
参考文件：
    drivers/mtd/nand/s3c2410.c
    drivers/mtd/nand/at91_nand.c
**************************************************/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/clk.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>

#include <asm/io.h>

#include <asm/arch/regs-nand.h>
#include <asm/arch/nand.h>

static struct nand_chip *s3c_nand;
static struct mtd_info *s3c_mtd;

static void NAND_SelectChip(struct mtd_info *mtd, int chipnr)
{
    if(chipnr == -1)
    {
        //取消选中
    }
    else
    {
        //选中
    }
}

static void NAND_CmdCtrl(struct mtd_info *mtd, int cmd,
                          unsigned int ctrl)
{
    if (ctrl & NAND_CLE)
    {
        //发命令
    }
    else
    {
        //发地址

    }
}

static int NAND_DevReady(struct mtd_info *mtf)
{
    return "NFSTAT[0]";
}

static int NAND_Init()
{
    //1. 分配一个nand_chip结构体
    s3c_nand = kzalloc(sizeof(struct nand_chip));

    //2.设置
    s3c_nand->select_chip = NAND_SelectChip;
    s3c_nand->cmd_ctrl = NAND_CmdCtrl;
    s3c_nand->IO_ADDR_R =;
    s3c_nand->IO_ADDR_R =;
    s3c_nand->dev_ready = NAND_DevReady;

    //3. 硬件相关操作

    //4. 使用
    s3c_mtd = kzalloc(sizeof(struct mtd_info));
    s3c_mtd->owner = THIS_MODULE;
    s3c_mtd->priv = s3c_nand;

    nand_scan(s3c_mtd, 1);

    //5. add_mtd_partitions
    return 0;
}

static void NAND_Exit()
{

}

module_init(NAND_Init);
module_exit(NAND_Exit);

MODULES_LICENSE("GPL");