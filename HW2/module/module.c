#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/version.h>

#include "module.h"

#define IOM_FND_ADDRESS 0x08000004
#define IOM_LED_ADDRESS 0x08000016
#define IOM_FPGA_DOT_ADDRESS 0x08000210
#define IOM_FPGA_TEXT_LCD_ADDRESS 0x08000090

static int fpga_port_usage = 0;
static unsigned char *iom_fpga_led_addr;
static unsigned char *iom_fpga_dot_addr;
static unsigned char *iom_fpga_fnd_addr;
static unsigned char *iom_fpga_text_lcd_addr;

int iom_dev_open(struct inode *minode, struct file *mfile);
int iom_dev_release(struct inode *minode, struct file *mfile);
static long iom_dev_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param);

struct file_operations iom_dev_fops =
{
	.owner			=	THIS_MODULE,
	.open			=	iom_dev_open,
	.unlocked_ioctl	=	iom_dev_ioctl,
	.release		=	iom_dev_release,
};

int iom_dev_open(struct inode *minode, struct file *mfile) 
{
	if(fpga_port_usage != 0) return -EBUSY;

	fpga_port_usage = 1;

	return 0;
}

int iom_dev_release(struct inode *minode, struct file *mfile) 
{
	fpga_port_usage = 0;

	return 0;
}

static long iom_dev_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
	printk("num : %d\n",ioctl_num);
	printk("param : %ld\n",ioctl_param);
	/*
	switch(ioctl_num)
	{
		case IOCTL_FND :
			printf("param %d\n",ioctl_num);
			printf("param %ld\n",ioctl_param);
			break;
		case IOCTL_LED :
			break;
		case IOCTL_DOT :
			break;
		case IOCTL_TEXT_LCD :
			break;
	}
	*/

	return 0;
}

int __init iom_dev_init(void)
{
	int result;

	result = register_chrdev(IOM_DEV_MAJOR, IOM_DEV_NAME, &iom_dev_fops);
	if(result < 0) {
		printk(KERN_WARNING"Can't get any major\n");
		return result;
	}

	iom_fpga_led_addr = ioremap(IOM_LED_ADDRESS, 0x1);
	iom_fpga_fnd_addr = ioremap(IOM_FND_ADDRESS, 0x4);
	iom_fpga_dot_addr = ioremap(IOM_FPGA_DOT_ADDRESS, 0x10);
	iom_fpga_text_lcd_addr = ioremap(IOM_FPGA_TEXT_LCD_ADDRESS, 0x32);

	printk("init module, %s major number : %d\n", IOM_DEV_NAME, IOM_DEV_MAJOR);

	return 0;
}

void __exit iom_dev_exit(void)
{ 
	iounmap(iom_fpga_led_addr);
	iounmap(iom_fpga_fnd_addr);
	iounmap(iom_fpga_dot_addr);
	iounmap(iom_fpga_text_lcd_addr);
	unregister_chrdev(IOM_DEV_MAJOR, IOM_DEV_NAME);
}

module_init ( iom_dev_init );
module_exit ( iom_dev_exit );
//MODULE_LICENSE("Dual BSD/GPL");
MODULE_LICENSE("GPL");