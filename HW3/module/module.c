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

//static int fpga_port_usage = 0;
static unsigned char *iom_fpga_fnd_addr;
struct file *global_inode;
char data_fnd[4]={0,0,0,0};

static int inter_major=0, inter_minor=0;
static int result;
static dev_t inter_dev;
static struct cdev inter_cdev;
static int inter_open(struct inode *, struct file *);
static int inter_release(struct inode *, struct file *);
static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);

irqreturn_t inter_handler1(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_handler2(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_handler3(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_handler4(int irq, void* dev_id, struct pt_regs* reg);
ssize_t iom_fpga_fnd_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t kernel_timer_write();
static void kernel_timer_blink(unsigned long timeout);
void end(void);

struct struct_timer {
	struct timer_list timer;
	int count;
};

struct struct_timer mydata;
//static inter_usage=0;
int interruptCount=0;
DECLARE_WAIT_QUEUE_HEAD(my_queue);

static struct file_operations inter_fops =
{
	.open = inter_open,
	.write = inter_write,
	.release = inter_release,
};

irqreturn_t inter_handler1(int irq, void* dev_id, struct pt_regs* reg) {
	printk(KERN_ALERT "interrupt1!!! = %x\n", gpio_get_value(IMX_GPIO_NR(1, 11)));
	//kernel_timer_write(global_inode, NULL, 1, 0);
	return IRQ_HANDLED;
}

irqreturn_t inter_handler2(int irq, void* dev_id, struct pt_regs* reg) {
  printk(KERN_ALERT "interrupt2!!! = %x\n", gpio_get_value(IMX_GPIO_NR(1, 12)));

	//__wake_up(&my_queue, 1, 1, NULL);
  return IRQ_HANDLED;
}

irqreturn_t inter_handler3(int irq, void* dev_id,struct pt_regs* reg) {
	printk(KERN_ALERT "interrupt3!!! = %x\n", gpio_get_value(IMX_GPIO_NR(2, 15)));

	return IRQ_HANDLED;
}

irqreturn_t inter_handler4(int irq, void* dev_id, struct pt_regs* reg) {
	printk(KERN_ALERT "interrupt4!!! = %x\n", gpio_get_value(IMX_GPIO_NR(5, 14)));
	
	return IRQ_HANDLED;
}


static int inter_open(struct inode *minode, struct file *mfile){
	int ret;
	int irq;
	interruptCount = 0;

	printk(KERN_ALERT "Open Module\n");

	// int1
	gpio_direction_input(IMX_GPIO_NR(1,11));
	irq = gpio_to_irq(IMX_GPIO_NR(1,11));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq,inter_handler1,IRQF_TRIGGER_RISING,"home",0);

	// int2
	gpio_direction_input(IMX_GPIO_NR(1,12));
	irq = gpio_to_irq(IMX_GPIO_NR(1,12));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq,inter_handler2,IRQF_TRIGGER_RISING,"back",0);

	// int3
	gpio_direction_input(IMX_GPIO_NR(2,15));
	irq = gpio_to_irq(IMX_GPIO_NR(2,15));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq,inter_handler3,IRQF_TRIGGER_RISING,"vol+",0);

	// int4
	gpio_direction_input(IMX_GPIO_NR(5,14));
	irq = gpio_to_irq(IMX_GPIO_NR(5,14));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq,inter_handler4,IRQF_TRIGGER_RISING,"vol-",0);

	return 0;
}

static int inter_release(struct inode *minode, struct file *mfile){
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 11)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 12)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(2, 15)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(5, 14)), NULL);
	
	printk(KERN_ALERT "Release Module\n");
	return 0;
}

static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos ){
	printk("write\n");
	global_inode = filp;
	interruptible_sleep_on(&my_queue); //sleep
	return 0;
}

ssize_t iom_fpga_fnd_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) 
{
	unsigned short int value_short = 0;
	global_inode = inode;

	value_short = gdata[0] << 12 | gdata[1] << 8 | gdata[2] << 4 | gdata[3];
  outw(value_short,(unsigned int)iom_fpga_fnd_addr);	    

	return length;
}

ssize_t kernel_timer_write()
{
	mydata.count = 0;

	del_timer_sync(&mydata.timer);

	mydata.timer.expires = jiffies + (HZ);
	mydata.timer.data = (unsigned long)&mydata;
	mydata.timer.function	= kernel_timer_blink;

	add_timer(&mydata.timer);
	return 1;
}

static void kernel_timer_blink(unsigned long timeout)
 {
	data_fnd[3]++;
	if(data_fnd[3] == 5)
	{
		end();
		return;
	}
	
	iom_fpga_fnd_write(global_inode, data_fnd, 4, 0);

	mydata.timer.expires = get_jiffies_64() + (HZ);
	mydata.timer.data = (unsigned long)&mydata;
	mydata.timer.function = kernel_timer_blink;

	add_timer(&mydata.timer);
	return;
}

void end(void)
{
	char end_fnd[4];
	memset(end_fnd, 0, sizeof(end_fnd));
	iom_fpga_fnd_write(global_inode, end_fnd, 4, 0);
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

	printk(KERN_ALERT "Init Module Success \n");
	printk(KERN_ALERT "Device : /dev/stopwatch, Major Num : 242 \n");
	return 0;
}

static void __exit inter_exit(void) {
	del_timer_sync(&mydata.timer);
	cdev_del(&inter_cdev);
	unregister_chrdev_region(inter_dev, 1);
	iounmap(iom_fpga_fnd_addr);
	
	printk(KERN_ALERT "Remove Module Success \n");
}

module_init(inter_init);
module_exit(inter_exit);
MODULE_LICENSE("GPL");