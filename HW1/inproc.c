#include "main.h"

#define BUFF_SIZE 64
#define MAX_BUTTON 9
#define KEY_RELEASE 0
#define KEY_PRESS 1


void read_mod();

int main (int argc, char *argv[])
{
	read_mod();	
	return 0;
}

/*************************
function name : read_mod()
parameter : none
function content : input SW, MENU key and send them through message queue
*************************/
void read_mod()
{
	//struct msgbuf msg;
	struct input_event ev[BUFF_SIZE];	
	unsigned char push_sw_buff[MAX_BUTTON];
	int i, fd, rd, dev, size = sizeof (struct input_event);
	int queue_id = msgget(key,IPC_CREAT|0644);
	//char name[256] = "Unknown";

	char* menu = "/dev/input/event0";
	char* nine_key = "/dev/fpga_push_switch";

	if((fd = open(menu, O_RDONLY|O_NONBLOCK)) == -1)
	{
		printf("%s is not a vaild device.n", menu);
		return;
	}
	
	if((dev = open(nine_key, O_RDWR)) < 0)	
	{
		printf("%s is not a vaild device.n", dev);
		close(fd);
		return;
	}

	while (1){
		usleep(200000);

		struct msgbuf msg;
		msg.msgtype = 1;
		int flag = 0, value, code;
		msg.code = 0;
		
		if ((rd = read (fd, ev, size * BUFF_SIZE)) >= size)
		{			
			value = ev[0].value; // press, release
			code = ev[0].code; // button code

			if(value == KEY_PRESS)
			{
				flag = 1;
				msg.code = code;
			}
		}

		read(dev, &push_sw_buff, sizeof(push_sw_buff));

		for(i=0;i<MAX_BUTTON;i++)
		{
			msg.sw_data[i] = push_sw_buff[i];

			if(msg.sw_data[i] == 1)
				flag = 1;
		}
		
		if(flag == 1)
		{	
			int rtn = msgsnd(queue_id, (void*)&msg, sizeof(msg), IPC_NOWAIT);
			if(msg.code == 158)
			{
				close(fd);
				close(dev);
				return;
			}
		}
	}	

	close(fd);
	close(dev);
	return;
}
