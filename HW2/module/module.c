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
static unsigned char *iom_fpga_fnd_addr;
static unsigned char *iom_fpga_led_addr;
static unsigned char *iom_fpga_dot_addr;
static unsigned char *iom_fpga_text_lcd_addr;
int led_arr[9] = {0,128,64,32,16,8,4,2,1};

ssize_t iom_fpga_fnd_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t iom_fpga_led_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t iom_fpga_dot_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t iom_fpga_text_lcd_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);

int iom_dev_open(struct inode *minode, struct file *mfile);
int iom_dev_release(struct inode *minode, struct file *mfile);
static long iom_dev_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param);

struct struct_timer {
	struct timer_list timer;
	int count;
};

struct struct_timer tm;

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

ssize_t iom_fpga_fnd_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) 
{
	unsigned short int value_short = 0;
	
	value_short = gdata[0] << 12 | gdata[1] << 8 | gdata[2] << 4 | gdata[3];
  outw(value_short,(unsigned int)iom_fpga_fnd_addr);	    

	return length;
}

ssize_t iom_fpga_led_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) 
{
	unsigned short _s_value;

  _s_value = gdata[0];
  outw(_s_value, (unsigned int)iom_fpga_led_addr);

	return length;
}

ssize_t iom_fpga_dot_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) 
{
	int i;
	unsigned short int _s_value;

	for(i=0;i<length;i++)
    {
      _s_value = fpga_number[(int)gdata[0]][i];
			outw(_s_value,(unsigned int)iom_fpga_dot_addr+i*2);
    }
	
	return length;
}

ssize_t iom_fpga_text_lcd_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) 
{
	int i;
  unsigned short int _s_value = 0;

	for(i=0;i<length;i++)
  {
    _s_value = gdata[i] << 8 | gdata[i+1];
		outw(_s_value,(unsigned int)iom_fpga_text_lcd_addr+i);
		i++;
  }

	return length;
}

static long iom_dev_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
	int i, mask = 0xFF;
	char gdata[4];
	char data_fnd[4];
	char data_led[2];
	char data_dot[2];
	char data_lcd[32];
	char tmp[16];

	for(i=3;i>=0;i--)
	{
		gdata[i] = (char)(ioctl_param & mask);
		//printk("data[%d] : %d\n",i,gdata[i]);
		ioctl_param = ioctl_param >> 8;
	}

	data_led[0] = led_arr[(int)gdata[1]];
	data_dot[0] = gdata[1];
	
	for(i=0;i<4;i++)
	{
		if(i == gdata[0])		
		{
			data_fnd[i] = gdata[1];
		}
		else
			data_fnd[i] = 0;
	}

	strncpy(tmp, "20131574", 8);
	for(i=0;i<16;i++)
	{
		if(i < strlen(tmp) - 1)
			data_lcd[i] = tmp[i];
		else
			data_lcd[i] = ' ';
	}

	strncpy(tmp, "OhHyunSeok", 10);
	for(i=16;i<32;i++)
	{
		if(i < strlen(tmp) + 16)
			data_lcd[i] = tmp[i - 16];
		else
			data_lcd[i] = ' ';
	}

	switch(ioctl_num)
	{
		case IOCTL_FPGA :
			iom_fpga_fnd_write(file, data_fnd, 4, 0);
			iom_fpga_led_write(file, data_led, 1, 0);
			iom_fpga_dot_write(file, data_dot, 10, 0);
			iom_fpga_text_lcd_write(file, data_lcd, 32, 0);
			break;
	}
	
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

	init_timer(&(tm.timer));
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
	del_timer_sync(&tm.timer);
}

module_init ( iom_dev_init );
module_exit ( iom_dev_exit );
//MODULE_LICENSE("Dual BSD/GPL");
MODULE_LICENSE("GPL");