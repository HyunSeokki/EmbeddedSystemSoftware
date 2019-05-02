#include "main.h"

#define MAX_DIGIT 4
#define MAX_BUFF 32
#define LINE_BUFF 16
#define FPGA_BASE_ADDRESS 0x08000000
#define LED_ADDR 0x16
#define FND_DEVICE "/dev/fpga_fnd"
#define FPGA_TEXT_LCD_DEVICE "/dev/fpga_text_lcd"
#define FPGA_DOT_DEVICE "/dev/fpga_dot"

struct msgbuf msg;

void output_fnd();
void output_led();
void output_lcd();
void output_dot();

int main (int argc, char *argv[])
{
	int queue_id = msgget(key,IPC_CREAT|0644);

	while(1)
	{
		msg.code = 0;
		int rtn = msgrcv(queue_id,(void*)&msg,sizeof(msg),2,IPC_NOWAIT);
		
		if(msg.code == 158)
		{
			break;
		}

		if(rtn != -1)
		{
			output_fnd();
			output_led();
			output_lcd();
			output_dot();
		}
	}

	return 0;
}

/*************************
function name : output_fnd()
parameter : none
function content : output fnd data execute by main process
*************************/
void output_fnd()
{
	int dev, i, fd;
	unsigned char data[4];
	unsigned char retval;
	dev = open(FND_DEVICE, O_RDWR);

	if(dev < 0)
	{
		printf("Device open error : %s\n", FND_DEVICE);
		exit(1);
	}

	for(i=0;i<4;i++)
		data[i] = msg.fnd_data[i];
	
	retval = write(dev,&data,4);
	close(dev);

	return;
}

/*************************
function name : output_led()
parameter : none
function content : output led data execute by main process
*************************/
void output_led()
{
	int fd;
	unsigned long *fpga_addr = 0;
	unsigned char *led_addr = 0;

	fd = open("/dev/mem", O_RDWR | O_SYNC);

	if(fd < 0)
	{
		perror("dev/mem open error");
		exit(1);
	}

	fpga_addr = (unsigned long *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, FPGA_BASE_ADDRESS);

	if(fpga_addr == MAP_FAILED)
	{
		printf("mmap error!\n");
		close(fd);
		exit(1);		
	}
	
	led_addr = (unsigned char*)((void*)fpga_addr+LED_ADDR);
	*led_addr = msg.led_data;

	munmap(led_addr, 4096);
	close(fd);

	return;
}

/*************************
function name : output_lcd()
parameter : none
function content : output lcd data execute by main process
*************************/
void output_lcd()
{
	int i,dev;
	unsigned char string[32]="";

	dev = open(FPGA_TEXT_LCD_DEVICE, O_WRONLY);

	if(dev < 0)
	{
		printf("Device open error : %s\n", FPGA_TEXT_LCD_DEVICE);
		exit(1);
	}
	for(i=0;i<MAX_BUFF;i++)
		string[i]= msg.lcd_data[i];

	write(dev,string,MAX_BUFF);
	close(dev);

	return;
}

/*************************
function name : output_dot()
parameter : none
function content : output dot data execute by main process
*************************/
void output_dot()
{
	int dev;
	dev = open(FPGA_DOT_DEVICE, O_WRONLY);

	if(dev < 0)
	{
		printf("Device open error : %s\n", FPGA_DOT_DEVICE);
		exit(1);
	}

	write(dev, msg.dot_data, sizeof(msg.dot_data));
	close(dev);

	return;
}
