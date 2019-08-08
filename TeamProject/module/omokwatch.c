#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <asm/irq.h>
#include <mach/gpio.h>
#include <linux/platform_device.h>
#include <asm/gpio.h>
// #include <linux/sched.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/cdev.h>

/* Define stopwatch states */
#define STATE_RUNNING 0
#define STATE_PAUSED 1
#define STATE_INIT 2
#define FPGA_FND_ADDRESS 0x08000004 // physical address

/* struct for kernel timer */
struct struct_timer_data {
	struct timer_list timer;
 	int count;
} ;

/* driver variables */
static int inter_major=246, inter_minor=0;
static int result;
static dev_t inter_dev;
static struct cdev inter_cdev;
static int omokwatch_usage = 0;

/* fpga_fnd variables */
static unsigned char *fpga_fnd_addr;
unsigned char fnd_data[4];

/* timer variables */
struct struct_timer_data timer_sec;

static int inter_open(struct inode *, struct file *);
static int inter_release(struct inode *, struct file *);
static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
static int inter_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);

irqreturn_t inter_HOME_btn(int irq, void* dev_id, struct pt_regs* reg);

void set_fnd_data(int total_sec);
void initialize_device(void);
ssize_t fpga_fnd_write(unsigned char value[4], size_t length);
void set_timer_timer_sec(void);
static void timer_sec_periodic(unsigned long timeout);

static struct file_operations inter_fops =
{
	.owner = THIS_MODULE,
	.open = inter_open,
	.write = inter_write,
	.read = inter_read,
	.release = inter_release,
};

void set_fnd_data(int total_sec){

	int min = total_sec / 60;
	int sec = total_sec % 60;
	// printk(KERN_ALERT "min:sec = %d %d\n", min, sec);

	fnd_data[0] = min % 100 / 10; /* min value cannot exceed 100 */
	fnd_data[1] = min % 10;
	fnd_data[2] = sec / 10;
	fnd_data[3] = sec % 10;
}
void initialize_device(void){
	int i;
	/* initialize fpga_fnd */
	for(i=0; i<4; i++) fnd_data[i] = 0;
	fpga_fnd_write(fnd_data, 4);
	
	/* init status variable */

	/* Delete all timers */
	del_timer(&timer_sec.timer); /* Can not use del_timer_sync() in interrupt context */
}
irqreturn_t inter_HOME_btn(int irq, void* dev_id, struct pt_regs* reg) {
	set_timer_timer_sec();
	return IRQ_HANDLED;
}
static void timer_sec_periodic(unsigned long timeout) {

	timer_sec.count--;
	if(timer_sec.count > 0){
		timer_sec.timer.expires = get_jiffies_64() + (1 * HZ);
		timer_sec.timer.data = (unsigned long)&timer_sec;
		timer_sec.timer.function = timer_sec_periodic;
		add_timer(&timer_sec.timer);	  
	}

	set_fnd_data(timer_sec.count - 1);
	fpga_fnd_write(fnd_data, 4);
}
void set_timer_timer_sec(void){
    /* Can not use del_timer_sync() in interrupt context */
	del_timer(&timer_sec.timer);

	timer_sec.count = 11;
	timer_sec.timer.expires = get_jiffies_64() + (1 * HZ);
	timer_sec.timer.data = (unsigned long)&timer_sec;
	timer_sec.timer.function = timer_sec_periodic;
	add_timer(&timer_sec.timer);
	
	set_fnd_data(timer_sec.count -1);
	fpga_fnd_write(fnd_data, 4);
}
static int inter_open(struct inode *minode, struct file *mfile){
	int ret;
	int irq;
	printk(KERN_ALERT "Open Module\n");

	/* Check device usage */
	if (omokwatch_usage != 0) {
		return -EBUSY;
	}
	/* Assign default value to variables  */
	omokwatch_usage = 1;
	timer_sec.count = 11;

	/* Initialize some variables and timer */
	initialize_device();
	// inter_HOME_btn
	gpio_free(IMX_GPIO_NR(1,11));
	gpio_request(IMX_GPIO_NR(1,11), NULL); 
	gpio_direction_input(IMX_GPIO_NR(1,11));
	irq = gpio_to_irq(IMX_GPIO_NR(1,11));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, (void *)inter_HOME_btn, IRQF_TRIGGER_FALLING, "HOME", 0);

	return 0;
}

static int inter_release(struct inode *minode, struct file *mfile){
	omokwatch_usage = 0;
	initialize_device();
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 11)), NULL);
	printk(KERN_ALERT "Release Module\n");
	return 0;
}
ssize_t fpga_fnd_write(unsigned char value[4], size_t length) {
    unsigned short int value_short = 0;
    value_short = value[0] << 12 | value[1] << 8 |value[2] << 4 |value[3];
    outw(value_short,(unsigned int)fpga_fnd_addr);	    
	return length;
}
static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos ){
	set_timer_timer_sec();
	return 0;
}
static int inter_read(struct file *filp, char *buf, size_t count, loff_t *f_pos){
	return timer_sec.count;
}

static int inter_register_cdev(void)
{
	int error;
	if(inter_major) {
		inter_dev = MKDEV(inter_major, inter_minor);
		error = register_chrdev_region(inter_dev,1,"inter");
	}else{
		error = alloc_chrdev_region(&inter_dev,inter_minor,1,"inter");
		inter_major = MAJOR(inter_dev);
	}
	if(error<0) {
		printk(KERN_WARNING "inter: can't get major %d\n", inter_major);
		return result;
	}
	printk(KERN_ALERT "major number = %d\n", inter_major);
	cdev_init(&inter_cdev, &inter_fops);
	inter_cdev.owner = THIS_MODULE;
	inter_cdev.ops = &inter_fops;
	/* mmap fpga_fnd device address */
	fpga_fnd_addr = ioremap(FPGA_FND_ADDRESS, 0x4);

	error = cdev_add(&inter_cdev, inter_dev, 1);
	if(error) {
		printk(KERN_NOTICE "inter Register Error %d\n", error);
	}
	return 0;
}

static int __init inter_init(void) {
	int result;
	if((result = inter_register_cdev()) < 0 )
		return result;
	printk(KERN_ALERT "Init Module Success \n");
	printk(KERN_ALERT "Device : /dev/omokwatch, Major Num : 246 \n");
	init_timer(&timer_sec.timer);    
	return 0;
}

static void __exit inter_exit(void) {
	/* unmap fpga_fnd address */
	iounmap(fpga_fnd_addr);
	/* release stopwatch device */
	cdev_del(&inter_cdev);
	unregister_chrdev_region(inter_dev, 1);
	del_timer_sync(&timer_sec.timer);

	printk(KERN_ALERT "Remove Module Success \n");
}

module_init(inter_init);
module_exit(inter_exit);
MODULE_LICENSE("GPL");
