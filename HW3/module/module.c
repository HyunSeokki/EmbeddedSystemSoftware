#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <asm/irq.h>
#include <mach/gpio.h>
#include <linux/platform_device.h>
#include <asm/gpio.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/cdev.h>
//need comment
#define IOM_FPGA_FND_ADDRESS 0x08000004
#define IOM_DEV_MAJOR 242
#define IOM_DEV_NAME "/dev/stopwatch"

struct struct_timer {
	struct timer_list timer;
	int sec;
};

static unsigned char *iom_fpga_fnd_addr;

static int inter_minor=0;
static int result;
static dev_t inter_dev;
static struct cdev inter_cdev;
struct struct_timer mydata;
struct struct_timer myexit;
static int inter_usage=0;
int interruptCount=0;
int isPaused = 1;
int pause_flag = 0;
int elapsed = 0;
int next_expire = 0;
DECLARE_WAIT_QUEUE_HEAD(my_queue);

static int inter_open(struct inode *, struct file *);
static int inter_release(struct inode *, struct file *);
static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);

irqreturn_t inter_handler1(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_handler2(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_handler3(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_handler4(int irq, void* dev_id, struct pt_regs* reg);

ssize_t kernel_timer_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
static void kernel_timer_stopwatch(unsigned long timeout);
static void kernel_timer_wake(unsigned long timeout);

ssize_t iom_fpga_fnd_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);

void display_zero(void);

static struct file_operations inter_fops =
{
	.open = inter_open,
	.write = inter_write,
	.release = inter_release,
};

irqreturn_t inter_handler1(int irq, void* dev_id, struct pt_regs* reg)
{
	if(isPaused == 1)
	{
		isPaused = 0;
		if(pause_flag == 1)
		{
			pause_flag = 0;
			mydata.timer.expires = get_jiffies_64() + (next_expire-elapsed);
			mydata.timer.data = (unsigned long)&mydata;
			mydata.timer.function = kernel_timer_stopwatch;

			add_timer(&mydata.timer);
		}
		else
		{			
			kernel_timer_write(NULL, NULL, 1, 0);
		}		
	}
	return IRQ_HANDLED;
}

irqreturn_t inter_handler2(int irq, void* dev_id, struct pt_regs* reg) 
{
	isPaused = 1;
	pause_flag = 1;
	del_timer_sync(&mydata.timer);

	return IRQ_HANDLED;
}

irqreturn_t inter_handler3(int irq, void* dev_id,struct pt_regs* reg)
{
	isPaused = 1;
	pause_flag = 0;
	mydata.sec = 0;

	del_timer_sync(&mydata.timer);
	display_zero();
	return IRQ_HANDLED;
}

irqreturn_t inter_handler4(int irq, void* dev_id, struct pt_regs* reg) 
{
	myexit.sec = 0;
	if(gpio_get_value(IMX_GPIO_NR(5, 14)) == 1)
		del_timer_sync(&myexit.timer);
	else
	{
		myexit.timer.expires = get_jiffies_64() + 3 * HZ;
		myexit.timer.data = (unsigned long)&myexit;
		myexit.timer.function = kernel_timer_wake;

		add_timer(&myexit.timer);
	}

	return IRQ_HANDLED;
}


static int inter_open(struct inode *minode, struct file *mfile)
{
	int ret;
	int irq;
	interruptCount = 0;

	if(inter_usage != 0) return -EBUSY;
	inter_usage = 1;

	printk(KERN_ALERT "Open Module\n");

	// int1
	gpio_direction_input(IMX_GPIO_NR(1,11));
	irq = gpio_to_irq(IMX_GPIO_NR(1,11));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq,(void*)inter_handler1,IRQF_TRIGGER_RISING,"home",0);

	// int2
	gpio_direction_input(IMX_GPIO_NR(1,12));
	irq = gpio_to_irq(IMX_GPIO_NR(1,12));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq,(void*)inter_handler2,IRQF_TRIGGER_RISING,"back",0);

	// int3
	gpio_direction_input(IMX_GPIO_NR(2,15));
	irq = gpio_to_irq(IMX_GPIO_NR(2,15));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq,(void*)inter_handler3,IRQF_TRIGGER_RISING,"vol+",0);

	// int4
	gpio_direction_input(IMX_GPIO_NR(5,14));
	irq = gpio_to_irq(IMX_GPIO_NR(5,14));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq,(void*)inter_handler4,IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,"vol-",0);

	return 0;
}

static int inter_release(struct inode *minode, struct file *mfile)
{
	inter_usage = 0;

	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 11)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 12)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(2, 15)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(5, 14)), NULL);

	return 0;
}

static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos )
{
	interruptible_sleep_on(&my_queue); //sleep
	return 0;
}

ssize_t iom_fpga_fnd_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) 
{
	unsigned short int value_short = 0;
	unsigned char value[4];
	unsigned char min, sec;
	min = mydata.sec/60;
	sec = mydata.sec%60;

	value[0] = min / 10;
	value[1] = min % 10;
	value[2] = sec / 10;
	value[3] = sec % 10;

	value_short = value[0] << 12 | value[1] << 8 | value[2] << 4 | value[3];
	outw(value_short,(unsigned int)iom_fpga_fnd_addr);	    

	return length;
}

ssize_t kernel_timer_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what)
{
	del_timer_sync(&mydata.timer);	

	//mydata.timer.expires = jiffies + (HZ);
	mydata.timer.expires = get_jiffies_64() + (HZ);
	mydata.timer.data = (unsigned long)&mydata;
	mydata.timer.function	= kernel_timer_stopwatch;

	add_timer(&mydata.timer);
	return length;
}

static void kernel_timer_stopwatch(unsigned long timeout)
{
	struct struct_timer *pdata = (struct struct_timer*)timeout;
	if(isPaused)
	{
		elapsed = get_jiffies_64();
		next_expire = mydata.timer.expires;
	}
	pdata->sec++;
	if(pdata->sec == 3600)
		pdata->sec = 0;

	iom_fpga_fnd_write(NULL, NULL, 4, 0);

	mydata.timer.expires = get_jiffies_64() + (HZ);
	mydata.timer.data = (unsigned long)&mydata;
	mydata.timer.function = kernel_timer_stopwatch;

	add_timer(&mydata.timer);
	return;
}

static void kernel_timer_wake(unsigned long timeout)
{
	isPaused = 1;
	mydata.sec = 0;
	del_timer_sync(&mydata.timer);
	display_zero();
	__wake_up(&my_queue, 1, 1, NULL);
	return;
}

void display_zero(void)
{
	unsigned short int value_short = 0;
	value_short = 0 << 12 | 0 << 8 | 0 << 4 | 0;
	outw(value_short,(unsigned int)iom_fpga_fnd_addr);	 
	return;
}

static int inter_register_cdev(void)
{
	int error;	
	inter_dev = MKDEV(IOM_DEV_MAJOR, inter_minor);
	error = register_chrdev_region(inter_dev,1,"inter");

	if(error<0) {
		printk(KERN_WARNING "inter: can't get major %d\n", IOM_DEV_MAJOR);
		return result;
	}

	printk(KERN_ALERT "major number = %d\n", IOM_DEV_MAJOR);
	cdev_init(&inter_cdev, &inter_fops);
	inter_cdev.owner = THIS_MODULE;
	inter_cdev.ops = &inter_fops;
	error = cdev_add(&inter_cdev, inter_dev, 1);

	if(error)
	{
		printk(KERN_NOTICE "inter Register Error %d\n", error);
	}

	return 0;
}

static int __init inter_init(void) {
	int result;
	if((result = inter_register_cdev()) < 0 )
		return result;

	iom_fpga_fnd_addr = ioremap(IOM_FPGA_FND_ADDRESS, 0x4);

	init_timer(&(mydata.timer));
	init_timer(&(myexit.timer));

	printk(KERN_ALERT "Init Module Success \n");
	printk(KERN_ALERT "Device : /dev/stopwatch, Major Num : 242 \n");
	return 0;
}

static void __exit inter_exit(void) {
	cdev_del(&inter_cdev);
	unregister_chrdev_region(inter_dev, 1);
	iounmap(iom_fpga_fnd_addr);

	printk(KERN_ALERT "Remove Module Success \n");
}

module_init(inter_init);
module_exit(inter_exit);
MODULE_LICENSE("GPL");