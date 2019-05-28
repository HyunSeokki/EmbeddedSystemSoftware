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

#define IOM_FPGA_FND_ADDRESS 0x08000004
#define IOM_DEV_MAJOR 242
#define IOM_DEV_NAME "/dev/stopwatch"

struct struct_timer {
	struct timer_list timer;
	int sec;
};

//variable
static unsigned char *iom_fpga_fnd_addr;
static dev_t inter_dev;
static struct cdev inter_cdev;
struct struct_timer mydata;
struct struct_timer myexit;
static int inter_usage=0;
int isPaused = 0;
int start_flag = 0;
int elapsed = 0;
int next_expire = 0;
DECLARE_WAIT_QUEUE_HEAD(my_queue);

//function
static int inter_open(struct inode *, struct file *);
static int inter_release(struct inode *, struct file *);
static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);

irqreturn_t inter_home_handler(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_back_handler(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_volP_handler(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_volM_handler(int irq, void* dev_id, struct pt_regs* reg);

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

/***********************************************************
name : inter_home_handler
type : irqreturn_t
parameter : int irq, void* dev_id, struct pt_regs* reg
content : start timer
 ***********************************************************/
irqreturn_t inter_home_handler(int irq, void* dev_id, struct pt_regs* reg)
{
	if(start_flag == 0)
	{
		kernel_timer_write(NULL, NULL, 1, 0);
		start_flag = 1;
	}

	return IRQ_HANDLED;
}

/***********************************************************
name : inter_back_handler
type : irqreturn_t
parameter : int irq, void* dev_id, struct pt_regs* reg
content : pause or restart timer
 ***********************************************************/
irqreturn_t inter_back_handler(int irq, void* dev_id, struct pt_regs* reg) 
{
	if(start_flag == 1)
	{
		if(isPaused == 1)
		{
			mydata.timer.expires = get_jiffies_64() + (next_expire-elapsed);
			mydata.timer.data = (unsigned long)&mydata;
			mydata.timer.function = kernel_timer_stopwatch;

			add_timer(&mydata.timer);
		}
		else
			del_timer_sync(&mydata.timer);

		isPaused = !isPaused;	
	}

	return IRQ_HANDLED;
}

/***********************************************************
name : inter_volP_handler
type : irqreturn_t
parameter : int irq, void* dev_id, struct pt_regs* reg
content : reset timer
 ***********************************************************/
irqreturn_t inter_volP_handler(int irq, void* dev_id,struct pt_regs* reg)
{
	mydata.sec = 0;
	display_zero();
	del_timer_sync(&mydata.timer);

	if(isPaused == 0)
		kernel_timer_write(NULL, NULL, 1, 0);

	return IRQ_HANDLED;
}

/***********************************************************
name : inter_volM_handler
type : irqreturn_t
parameter : int irq, void* dev_id, struct pt_regs* reg
content : when user push vol- button during 3 seconds, end user application 
 ***********************************************************/
irqreturn_t inter_volM_handler(int irq, void* dev_id, struct pt_regs* reg) 
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

/***********************************************************
name : inter_open
type : static int
parameter : struct inode *minode, struct file *mfile
content : GPIO input set, Register interrupt service function
 ***********************************************************/
static int inter_open(struct inode *minode, struct file *mfile)
{
	int ret;
	int irq;

	if(inter_usage != 0) return -EBUSY;
	inter_usage = 1;

	printk(KERN_ALERT "Open Module\n");

	// Home button
	gpio_direction_input(IMX_GPIO_NR(1,11)); // GPIO input set
	irq = gpio_to_irq(IMX_GPIO_NR(1,11)); // return interrupt address
	ret=request_irq(irq,(void*)inter_home_handler,IRQF_TRIGGER_RISING,"home",0); // Register interrupt service function, when button pulled

	// Back button
	gpio_direction_input(IMX_GPIO_NR(1,12)); // GPIO input set
	irq = gpio_to_irq(IMX_GPIO_NR(1,12)); // return interrupt address
	ret=request_irq(irq,(void*)inter_back_handler,IRQF_TRIGGER_RISING,"back",0); // Register interrupt service function, when button pulled

	// Vol+ button
	gpio_direction_input(IMX_GPIO_NR(2,15)); // GPIO input set
	irq = gpio_to_irq(IMX_GPIO_NR(2,15)); // return interrupt address
	ret=request_irq(irq,(void*)inter_volP_handler,IRQF_TRIGGER_RISING,"vol+",0); // Register interrupt service function, when button pulled 

	// Vol- button
	gpio_direction_input(IMX_GPIO_NR(5,14)); // GPIO input set
	irq = gpio_to_irq(IMX_GPIO_NR(5,14)); // return interrupt address
	ret=request_irq(irq,(void*)inter_volM_handler,IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,"vol-",0); // Register interrupt service function, when button pushed and pulled

	return 0;
}

/***********************************************************
name : inter_release
type : static int
parameter : struct inode *minode, struct file *mfile
content : remove interrupt that sets request_irq function
 ***********************************************************/
static int inter_release(struct inode *minode, struct file *mfile)
{
	inter_usage = 0;

	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 11)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 12)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(2, 15)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(5, 14)), NULL);

	return 0;
}

/***********************************************************
name : inter_write
type : static int
parameter : struct file *filp, const char *buf, size_t count, loff_t *f_pos
content : sleep process
 ***********************************************************/
static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	interruptible_sleep_on(&my_queue); //sleep
	return 0;
}

/***********************************************************
name : iom_fpga_fnd_write
type : ssize_t
parameter : struct file *inode, const char *gdata, size_t length, loff_t *off_what
content : display timer on fnd
 ***********************************************************/
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

/***********************************************************
name : kernel_timer_write
type : ssize_t
parameter : struct file *inode, const char *gdata, size_t length, loff_t *off_what
content : call kernel_timer_stopwatch
 ***********************************************************/
ssize_t kernel_timer_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what)
{
	mydata.timer.expires = get_jiffies_64() + (HZ);
	mydata.timer.data = (unsigned long)&mydata;
	mydata.timer.function = kernel_timer_stopwatch;

	add_timer(&mydata.timer);

	return length;
}

/***********************************************************
name : kernel_timer_stopwatch
type : static void
parameter : unsigned long timeout
content : increase timer and display timer on fnd
 ***********************************************************/
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

/***********************************************************
name : kernel_timer_wake
type : static void
parameter : unsigned long timeout
content : initialize variable and display 0000 on fnd, wake process
 ***********************************************************/
static void kernel_timer_wake(unsigned long timeout)
{
	isPaused = 0;
	start_flag = 0;
	elapsed = 0;
	next_expire = 0;
	mydata.sec = 0;
	del_timer_sync(&mydata.timer);
	display_zero();
	__wake_up(&my_queue, 1, 1, NULL);

	return;
}

/***********************************************************
name : display_zero
type : void
parameter : void
content : display 0000 on fnd
 ***********************************************************/
void display_zero(void)
{
	unsigned short int value_short = 0;
	value_short = 0 << 12 | 0 << 8 | 0 << 4 | 0;
	outw(value_short,(unsigned int)iom_fpga_fnd_addr);	 
	return;
}

/***********************************************************
name : inter_register_cdev
type : static int
parameter : void
content : register interrupt
 ***********************************************************/
static int inter_register_cdev(void)
{
	int error;	
	inter_dev = MKDEV(IOM_DEV_MAJOR, 0); // get dev_t using major number and minor number
	error = register_chrdev_region(inter_dev,1,"inter"); // assign device

	if(error<0) {
		printk(KERN_WARNING "inter: can't get major %d\n", IOM_DEV_MAJOR);
		return -1;
	}

	printk(KERN_ALERT "major number = %d\n", IOM_DEV_MAJOR);
	cdev_init(&inter_cdev, &inter_fops); // initialize char device
	inter_cdev.owner = THIS_MODULE;
	inter_cdev.ops = &inter_fops;
	error = cdev_add(&inter_cdev, inter_dev, 1);

	if(error)
	{
		printk(KERN_NOTICE "inter Register Error %d\n", error);
	}

	return 0;
}

/***********************************************************
name : inter_init
type : static int
parameter : void
content : assign fnd address and init timer
 ***********************************************************/
static int __init inter_init(void)
{
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

/***********************************************************
name : inter_exit
type : static void
parameter : void
content : unassign fnd address and unregister interrupt
 ***********************************************************/
static void __exit inter_exit(void)
{
	cdev_del(&inter_cdev);
	unregister_chrdev_region(inter_dev, 1);
	iounmap(iom_fpga_fnd_addr);

	printk(KERN_ALERT "Remove Module Success \n");
}

module_init(inter_init);
module_exit(inter_exit);
MODULE_LICENSE("GPL");
