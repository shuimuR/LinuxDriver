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

struct s3c_nand_regs {
    unsigned long nfconf  ;
    unsigned long nfcont  ;
    unsigned long nfcmd   ;
    unsigned long nfaddr  ;
    unsigned long nfdata  ;
    unsigned long nfeccd0 ;
    unsigned long nfeccd1 ;
    unsigned long nfeccd  ;
    unsigned long nfstat  ;
    unsigned long nfestat0;
    unsigned long nfestat1;
    unsigned long nfmecc0 ;
    unsigned long nfmecc1 ;
    unsigned long nfsecc  ;
    unsigned long nfsblk  ;
    unsigned long nfeblk  ;
};

static struct s3c_nand_regs *s3c_nand_regs;

static struct mtd_partition s3c_nand_parts[] = {
        [0] = {
                .name   = "bootloader",
                .size   = 0x00040000,
                .offset	= 0,
        },
        [1] = {
                .name   = "params",
                .offset = MTDPART_OFS_APPEND,
                .size   = 0x00020000,
        },
        [2] = {
                .name   = "kernel",
                .offset = MTDPART_OFS_APPEND,
                .size   = 0x00200000,
        },
        [3] = {
                .name   = "root",
                .offset = MTDPART_OFS_APPEND,
                .size   = MTDPART_SIZ_FULL,
        }
};

static void NAND_SelectChip(struct mtd_info *mtd, int chipnr)
{
    if(chipnr == -1)
    {
        //取消选中
        s3c_nand_regs->nfcont |= (1 << 1);
    }
    else
    {
        //选中
        s3c_nand_regs->nfcont &= ~(1 << 1);
    }
}

static void NAND_CmdCtrl(struct mtd_info *mtd, int cmd,
                          unsigned int ctrl)
{
    if (ctrl & NAND_CLE)
    {
        //发命令
        s3c_nand_regs->nfcmd = cmd;
    }
    else
    {
        //发地址
        s3c_nand_regs->nfaddr = cmd;
    }
}

static int NAND_DevReady(struct mtd_info *mtf)
{
    return (s3c_nand_regs->nfstat & (1 << 0));
}

static int NAND_Init(void)
{
    struct clk * clk;

    s3c_nand_regs = ioremap(0x4E000000, sizeof(struct s3c_nand_regs));

    //1. 分配一个nand_chip结构体
    s3c_nand = kzalloc(sizeof(struct nand_chip), GFP_KERNEL);

    //2.设置
    s3c_nand->select_chip = NAND_SelectChip;
    s3c_nand->cmd_ctrl = NAND_CmdCtrl;
    s3c_nand->IO_ADDR_R = &s3c_nand_regs->nfdata;
    s3c_nand->IO_ADDR_W = &s3c_nand_regs->nfdata;
    s3c_nand->dev_ready = NAND_DevReady;
    s3c_nand->ecc.mode = NAND_ECC_SOFT;

    //3. 硬件相关操作
    clk = clk_get(NULL, "nand");
    clk_enable(clk);                //设置clk

    //HCLK = 100MHz
#define TACLS   0
#define TWRPH0  1
#define TWRPH1  0
    s3c_nand_regs->nfconf = (TACLS << 12) | (TWRPH0 << 8) | (TWRPH1 << 4);

    //使能nand控制器
    s3c_nand_regs->nfcont = (1 << 1) | ( 1 << 0);

    //4. 使用
    s3c_mtd = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
    s3c_mtd->owner = THIS_MODULE;
    s3c_mtd->priv = s3c_nand;

    nand_scan(s3c_mtd, 1);

    //5. add_mtd_partitions
    add_mtd_partitions(s3c_mtd, s3c_nand_parts, 4);

    return 0;
}

static void NAND_Exit(void)
{
    kfree(s3c_mtd);
    iounmap(s3c_nand_regs);
    kfree(s3c_nand);
    del_mtd_partitions(s3c_mtd);
}

module_init(NAND_Init);
module_exit(NAND_Exit);

MODULE_LICENSE("GPL");