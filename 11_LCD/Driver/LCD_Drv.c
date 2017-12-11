#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/clk.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>

#include <asm/mach/map.h>
#include <asm/arch/regs-lcd.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/fb.h>

static int s3c_lcdfb_setcolreg(unsigned int regno, unsigned int red,
                               unsigned int green, unsigned int blue,
                               unsigned int transp, struct fb_info *info);

static struct fb_ops My_LCD_Ops =
        {
                .owner = THIS_MODULE,
                .fb_setcolreg = s3c_lcdfb_setcolreg,
                .fb_fillrect = cfb_fillrect,
                .fb_copyarea = cfb_copyarea,
                .fb_imageblit = cfb_imageblit,
        };

static struct fb_info *s3c_lcd;
static volatile unsigned long *gpbcon;
static volatile unsigned long *gpbdat;
static volatile unsigned long *gpccon;
static volatile unsigned long *gpcdat;
static volatile unsigned long *gpdcon;
static volatile unsigned long *gpgcon;
static volatile unsigned long *gpgdat;

//as build, we need the wrong pallte
static u32 pseudo_palette[16];
/* from pxafb.c */
static inline unsigned int chan_to_field(unsigned int chan, struct fb_bitfield *bf)
{
    chan &= 0xffff;
    chan >>= 16 - bf->length;
    return chan << bf->offset;
}


static int s3c_lcdfb_setcolreg(unsigned int regno, unsigned int red,
                               unsigned int green, unsigned int blue,
                               unsigned int transp, struct fb_info *info)
{
    unsigned int val;

    if (regno > 16)
        return 1;

    /* 用red,green,blue三原色构造出val */
    val  = chan_to_field(red,	&info->var.red);
    val |= chan_to_field(green, &info->var.green);
    val |= chan_to_field(blue,	&info->var.blue);

    //((u32 *)(info->pseudo_palette))[regno] = val;
    pseudo_palette[regno] = val;
    return 0;
}

struct lcd_regs {
    unsigned long	lcdcon1;
    unsigned long	lcdcon2;
    unsigned long	lcdcon3;
    unsigned long	lcdcon4;
    unsigned long	lcdcon5;
    unsigned long	lcdsaddr1;
    unsigned long	lcdsaddr2;
    unsigned long	lcdsaddr3;
    unsigned long	redlut;
    unsigned long	greenlut;
    unsigned long	bluelut;
    unsigned long	reserved[9];
    unsigned long	dithmode;
    unsigned long	tpal;
    unsigned long	lcdintpnd;
    unsigned long	lcdsrcpnd;
    unsigned long	lcdintmsk;
    unsigned long	lpcsel;
};

static struct lcd_regs *LCD_Regs = NULL;

static int LCD_Init(void)
{
    //分配一个fb_info
    s3c_lcd = framebuffer_alloc(0, NULL);
    //设置
    //2.1设置固定的参数
    strcpy(s3c_lcd->fix.id, "MyLcd");
    s3c_lcd->fix.smem_len = 480*272*16/8;
    s3c_lcd->fix.type = FB_TYPE_PACKED_PIXELS;
    s3c_lcd->fix.visual = FB_VISUAL_TRUECOLOR;
    s3c_lcd->fix.line_length = 480*2;

    //2.2设置可变的参数
    s3c_lcd->var.xres = 480;
    s3c_lcd->var.yres = 272;
    s3c_lcd->var.xres_virtual = 480;
    s3c_lcd->var.yres_virtual = 272;
    s3c_lcd->var.bits_per_pixel = 16;

    //RGB 565
    s3c_lcd->var.red.offset = 11;
    s3c_lcd->var.red.length = 5;

    s3c_lcd->var.green.offset = 5;
    s3c_lcd->var.green.length = 6;

    s3c_lcd->var.blue.offset = 0;
    s3c_lcd->var.blue.length = 5;

    s3c_lcd->var.activate = FB_ACTIVATE_NOW;

    //2.3设置操作函数
    s3c_lcd->fbops = &My_LCD_Ops;

    //2.4其他的设置
    s3c_lcd->pseudo_palette = pseudo_palette;
    //s3c_lcd->screen_base = ;        //显存的虚拟地址
    s3c_lcd->screen_size = 480*272*16/8;

    //3硬件相关的操作
    //3.1配置GPIO 用于LCD
    gpbcon = (volatile unsigned long *)ioremap(0x56000010, 8);
    gpbdat = gpbcon + 1;
    gpccon = (volatile unsigned long *)ioremap(0x56000020, 8);
    gpcdat = gpdcon + 1;
    gpdcon = (volatile unsigned long *)ioremap(0x56000030, 8);
    gpgcon = (volatile unsigned long *)ioremap(0x56000060, 8);
    gpgdat = gpgcon + 1;

    *gpccon = 0xaaaaaaaa;
    *gpdcon = 0xaaaaaaaa;

    *gpbcon &= ~(3);
    *gpbcon |= 1;
    *gpbdat &= ~1;

    *gpgcon |= (3 << 8);

    //3.2根据LCD手册设置LCD控制器，如VCLK
    LCD_Regs = (volatile unsigned long *)ioremap(0x4D000000, sizeof(struct lcd_regs));
    LCD_Regs->lcdcon1 =  (4<<8) | (3 << 5) | (0x0C << 1);
    LCD_Regs->lcdcon2 = (1 << 24) | (271<<14) | (1<<6) | (9);
    LCD_Regs->lcdcon3 = (1<<19) | (479<<8) | (1);
    LCD_Regs->lcdcon4 = 40;
    LCD_Regs->lcdcon5 = (1 << 11) | (0 << 10) | (1 << 9) | (1 << 8) | (1 << 0);

    //3.3分配framebuffer，并把地址告诉LCD控制器
    s3c_lcd->screen_base = dma_alloc_writecombine(NULL, s3c_lcd->fix.smem_len, &s3c_lcd->fix.smem_start, GFP_KERNEL);

    LCD_Regs->lcdsaddr1 = (s3c_lcd->fix.smem_start >> 1) & ~(3 << 30);
    LCD_Regs->lcdsaddr2 = ((s3c_lcd->fix.smem_start + s3c_lcd->fix.smem_len) >> 1) & 0x1FFFFF;
    LCD_Regs->lcdsaddr3 = (480*16/16);

    //启动LCD
    LCD_Regs->lcdcon1 |= (1 << 0);          //enable LCD module
    LCD_Regs->lcdcon5 |= (1 << 3);          //enable PWR
    *gpbdat |= 1;               //open backlight

    //注册
    register_framebuffer(s3c_lcd);
    return 0;
}

static void LCD_Exit()
{
    unregister_framebuffer(s3c_lcd);
    LCD_Regs->lcdcon1 &= ~(1 << 0);
    *gpbdat &= ~1;

    dma_free_writecombine(NULL, s3c_lcd->fix.smem_len, s3c_lcd->screen_base, s3c_lcd->fix.smem_start);
    iounmap(LCD_Regs);
    iounmap(gpbcon);
    iounmap(gpccon);
    iounmap(gpgcon);
    iounmap(gpdcon);
    framebuffer_release(s3c_lcd);
}

module_init(LCD_Init);
module_exit(LCD_Exit);

MODULE_LICENSE("GPL");