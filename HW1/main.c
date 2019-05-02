#include "main.h"

int mode = 1;
int mode_fnd[4];

int mode1_modify = 0;
int mode1_flag = 0; 
int mode2_flag = 10;
int mode3_count = 0;
int mode3_dot = 0;
int mode3_prev_num;
int mode3_key_cnt = 0;
int mode3_len = 0;
unsigned char mode3_lcd[33];
int mode4_count = 0;
int mode4_cursor = 1;
int mode4_flag = 0;
int mode4_y = 0;
int mode4_x = 0x40;
unsigned char mode4_dot[10];
int mode5_count = 0;
int mode5_led = 128;
int mode5_rand[7] = {0x40,0x20,0x10,0x08,0x04,0x02,0x01};
int mode5_y[10];
int mode5_x[10];
int mode5_me = 0x08;
int mode5_gameover = 1;
int mode5_ddong_cnt = 0;

char mode3_sw[9][4] = {
	{'.','Q','Z'}, {'A','B','C'}, {'D','E','F'},
	{'G','H','I'}, {'J','K','L'}, {'M','N','O'},
	{'P','R','S'}, {'T','U','V'}, {'W','X','Y'}
};

unsigned char fpga_dot[3][10] = {
	{0,0,0,0,0,0,0,0,0,0},
	{0x1c,0x36,0x63,0x63,0x63,0x7f,0x7f,0x63,0x63,0x63},  // A
	{0x0c,0x1c,0x1c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x1e}   // 1
};

void init(int num);
void mode1();
void mode2();
void mode3();
void mode4();
void mode5();
void control_mode1(struct msgbuf);
void control_mode2(struct msgbuf);
void control_mode3(struct msgbuf);
void control_mode4(struct msgbuf);
void control_mode5(struct msgbuf);

int main()
{
	int i,inproc, outproc;
	int queue_id = msgget(key,IPC_CREAT|0644);
	char str[256];

	// Initialize
	srand(time(NULL));
	time(&current);
	t = localtime(&current);
	mode_fnd[0] = (t->tm_hour)/10;
	mode_fnd[1] = (t->tm_hour)%10;
	mode_fnd[2] = (t->tm_min)/10;
	mode_fnd[3] = (t->tm_min)%10;
	
	for(i=0;i<32;i++)
		mode3_lcd[i] = ' ';

	inproc = fork(); // fork input process
	if(inproc == -1)
	{
		perror("Input process fork error.\n");
		exit(1);
	}
	else if(inproc == 0)
	{
		char *argc[] = {"./inproc",str,NULL};
		execve(argc[0],argc,NULL);
	}
	else
	{
		outproc = fork(); // fork output process

		if(outproc == -1)
		{
			perror("Output process fork error.\n");
			exit(1);
		}
		else if(outproc == 0)
		{
			char *argc[] = {"./outproc",str,NULL};
			execve(argc[0],argc,NULL);
		}
		else
		{
			while(1)
			{
				usleep(200000);
				struct msgbuf msg;
				msg.code = 0;
				int rtn = msgrcv(queue_id,(void*)&msg,sizeof(msg),1,IPC_NOWAIT); // pop message queue

				if(mode == 1)
					mode1();
				else if(mode == 2)
					mode2();
				else if(mode == 3)
					mode3();
				else if(mode == 4)
					mode4();
				else if(mode == 5)
				{
					usleep(200000);
					mode5();
				}
										
				if(rtn != -1)
				{
					if(msg.code == 158) // Click back button
					{
						break;
					}
					else if(msg.code == 115) // Click VOL+ button
					{
						mode++;
						if(mode > 5)
							mode = 1;

						init(mode);
					}
					else if(msg.code == 114) // click VOL- button
					{
						mode--;
						if(mode < 1)
							mode = 5;
						
						init(mode);
					}

					else if(mode == 1)
						control_mode1(msg);
					else if(mode == 2)
						control_mode2(msg);
					else if(mode == 3)
						control_mode3(msg);
					else if(mode == 4)
						control_mode4(msg);
					else if(mode == 5)
						control_mode5(msg);
				}
			}
		}
	}
	return 0;
}

/*************************
function name : init()
parameter : none
function content : initialize variable
*************************/
void init(int num)
{
	int i;
	for(i=0;i<32;i++)
			mode3_lcd[i] = ' ';

	if(num == 1)
	{
		time(&current);
		t = localtime(&current);
		mode_fnd[0] = (t->tm_hour)/10;
		mode_fnd[1] = (t->tm_hour)%10;
		mode_fnd[2] = (t->tm_min)/10;
		mode_fnd[3] = (t->tm_min)%10;
		mode1_modify = 0;
		mode1_flag = 0;
		mode3_dot = 0;
	}
	else if(num == 2)
	{
		mode2_flag = 10;
		mode3_dot = 0;

		for(i=0;i<4;i++)
			mode_fnd[i] = 0;
	}
	else if(num == 3)
	{
		mode3_len = 0;
		mode3_dot = 1; 
		mode3_count = 0;
		mode3_prev_num = -1;
		mode3_key_cnt = 0;

		for(i=0;i<4;i++)
			mode_fnd[i] = 0;		
	}
	else if(num == 4)
	{
		mode4_count = 0;
		mode4_y = 0;
		mode4_x = 0x40;
		mode4_flag = 0;
		mode4_cursor = 1;

		for(i=0;i<10;i++)
			mode4_dot[i] = 0;

		for(i=0;i<4;i++)
			mode_fnd[i] = 0;
	}
	else if(num == 5)
	{
		mode5_count = 0;
		mode5_ddong_cnt = 0;
		mode5_me = 0x08;
		mode5_gameover = 1;		
	}
	return;
}

/*************************
function name : mode1()
parameter : none
function content : send output data and always running when mode1
*************************/
void mode1()
{
	int i,queue_id = msgget(key,IPC_CREAT|0644);
	struct msgbuf msg;

	msg.msgtype = 2;
	if(mode1_modify == 1) // when user push SW(1) - modify mode
	{
		if(mode1_flag == 0)
			msg.led_data = 128 + 32;
		else
			msg.led_data = 128 + 16;

		mode1_flag = !mode1_flag;
	}
	else if(mode1_modify == 0) // when user don't push SW(1)
	{
		time(&current);
		t = localtime(&current);
		mode_fnd[0] = (t->tm_hour)/10;
		mode_fnd[1] = (t->tm_hour)%10;
		mode_fnd[2] = (t->tm_min)/10;
		mode_fnd[3] = (t->tm_min)%10;

		msg.led_data = 128;
	}
	else if(mode1_modify == 2) // when user push SW(1) - not modify mode
	{
		msg.led_data = 128;
	}

	for(i=0;i<4;i++)
		msg.fnd_data[i] = mode_fnd[i];

	for(i=0;i<32;i++)
		msg.lcd_data[i] = mode3_lcd[i];
		
	for(i=0;i<10;i++)
		msg.dot_data[i] = fpga_dot[mode3_dot][i];	
		
	int rtn = msgsnd(queue_id, (void*)&msg, sizeof(msg), IPC_NOWAIT);

	return;
}

/*************************
function name : mode2()
parameter : none
function content : send output data and always running when mode2
*************************/
void mode2()
{
	int i,queue_id = msgget(key,IPC_CREAT|0644);
	struct msgbuf msg;

	msg.msgtype = 2;
	if(mode2_flag == 10) // decimal mode
		msg.led_data = 64;
	else if(mode2_flag == 8) // octal mode
		msg.led_data = 32;
	else if(mode2_flag == 4) // tetramal mode
		msg.led_data = 16;
	else if(mode2_flag == 2) // binary mode
		msg.led_data = 128;

	for(i=0;i<4;i++)
		msg.fnd_data[i] = mode_fnd[i];

	for(i=0;i<32;i++)
		msg.lcd_data[i] = mode3_lcd[i];

	for(i=0;i<10;i++)
		msg.dot_data[i] = fpga_dot[mode3_dot][i];

	int rtn = msgsnd(queue_id, (void*)&msg, sizeof(msg), IPC_NOWAIT);

	return;
}

/*************************
function name : mode3()
parameter : none
function content : send output data and always running when mode3
*************************/
void mode3()
{
	int i,queue_id = msgget(key,IPC_CREAT|0644);
	struct msgbuf msg;

	msg.msgtype = 2;
	msg.led_data = 0;	

	int tmp = mode3_count;
	for(i=3;i>=0;i--)
	{
		msg.fnd_data[i] = tmp % 10;
		tmp /= 10;
	}
	for(i=0;i<32;i++)
		msg.lcd_data[i] = mode3_lcd[i];

	for(i=0;i<10;i++)
		msg.dot_data[i] = fpga_dot[mode3_dot][i];

	int rtn = msgsnd(queue_id, (void*)&msg, sizeof(msg), IPC_NOWAIT);

	return;
}

/*************************
function name : mode4()
parameter : none
function content : send output data and always running when mode4
*************************/
void mode4()
{
	int i,queue_id = msgget(key,IPC_CREAT|0644);
	struct msgbuf msg;

	msg.msgtype = 2;
	msg.led_data = 0;	

	int tmp = mode4_count;
	for(i=3;i>=0;i--)
	{
		msg.fnd_data[i] = tmp % 10;
		tmp /= 10;
	}

	for(i=0;i<32;i++)
		msg.lcd_data[i] = mode3_lcd[i];

	for(i=0;i<10;i++)
		msg.dot_data[i] = mode4_dot[i];

	if(mode4_cursor == 1) // when user push SW(3) - cursor display mode
	{
		if(mode4_flag == 0)
			msg.dot_data[mode4_y] = msg.dot_data[mode4_y] ^ mode4_x;
		
		mode4_flag = !mode4_flag;
	}
	
	int rtn = msgsnd(queue_id, (void*)&msg, sizeof(msg), IPC_NOWAIT);

	return;
}

/*************************
function name : mode5()
parameter : none
function content : send output data and always running when mode5
*************************/
void mode5()
{
	int i,queue_id = msgget(key,IPC_CREAT|0644);
	int ddong = mode5_rand[rand()%7];
	struct msgbuf msg;

	msg.msgtype = 2;
	msg.led_data = mode5_led;
	mode5_led = mode5_led >> 1;	
	
	if(mode5_led == 0)
		mode5_led = 128;

	int tmp = mode5_count;
	for(i=3;i>=0;i--)
	{
		msg.fnd_data[i] = tmp % 10;
		tmp /= 10;
	}
	if(mode5_gameover == 1) // playing game
	{
		msg.lcd_data[0] = 'P';
		msg.lcd_data[1] = 'l';
		msg.lcd_data[2] = 'a';
		msg.lcd_data[3] = 'y';
		msg.lcd_data[4] = 'i';
		msg.lcd_data[5] = 'n';
		msg.lcd_data[6] = 'g';

		if(mode5_ddong_cnt < 10)
		{
			mode5_y[mode5_ddong_cnt] = 0;
			mode5_x[mode5_ddong_cnt] = ddong;
			mode5_ddong_cnt++;
		}
		else if(mode5_ddong_cnt == 10)
		{
			for(i=0;i<9;i++)
			{
				mode5_y[i] = mode5_y[i+1];
				mode5_x[i] = mode5_x[i+1];
			}	
			if(mode5_x[0] == mode5_me)
			{				
				mode5_gameover = 0;
				msg.lcd_data[0] = 'G';
				msg.lcd_data[1] = 'A';
				msg.lcd_data[2] = 'M';
				msg.lcd_data[3] = 'E';
				msg.lcd_data[4] = ' ';
				msg.lcd_data[5] = 'O';
				msg.lcd_data[6] = 'V';
				msg.lcd_data[7] = 'E';
				msg.lcd_data[8] = 'R';
			}
			else
			{							
				mode5_y[9] = 0;
				mode5_x[9] = ddong;
			}	
			mode5_count++;		
		}
		for(i=0;i<10;i++)
			msg.dot_data[i] = 0;

		for(i=0;i<mode5_ddong_cnt;i++)
			msg.dot_data[mode5_y[i]] = mode5_x[i];
		
		msg.dot_data[9] = mode5_me; // user location

		for(i=0;i<mode5_ddong_cnt;i++) // ddong down
		{
			if(mode5_y[i] != 9)
				mode5_y[i]++;
		}
	}
	else // game over
	{
		msg.lcd_data[0] = 'G';
		msg.lcd_data[1] = 'A';
		msg.lcd_data[2] = 'M';
		msg.lcd_data[3] = 'E';
		msg.lcd_data[4] = ' ';
		msg.lcd_data[5] = 'O';
		msg.lcd_data[6] = 'V';
		msg.lcd_data[7] = 'E';
		msg.lcd_data[8] = 'R';
	}
	int rtn = msgsnd(queue_id, (void*)&msg, sizeof(msg), IPC_NOWAIT);

	return;
}

/*************************
function name : control_mode1()
parameter : struct msgbuf
function content : If input received, control output data and send them when mode1
*************************/
void control_mode1(struct msgbuf msg)
{
	if(msg.sw_data[0] == 1) // switch mode
	{
		if(mode1_modify == 1)
			mode1_modify = 2;
		else
			mode1_modify = 1;

		mode1_flag = 0;
	}
	else if(msg.sw_data[1] == 1) // initialize
	{
		mode1_modify = 0;
		mode1_flag = 0;
	}
	else if(mode1_modify == 1 && (msg.sw_data[2] == 1 || msg.sw_data[3] == 1))
	{
		int sec = (mode_fnd[0] * 10 + mode_fnd[1]) * 3600 + (mode_fnd[2] * 10 + mode_fnd[3]) * 60;

		if(msg.sw_data[2] == 1)
			sec+=3600;
		else if(msg.sw_data[3] == 1)
			sec+=60;

		if(sec > 86400)
			sec -= 86400;

		int hour = sec / 3600;
		int min = (sec % 3600) / 60;

		mode_fnd[0] = hour / 10;
		mode_fnd[1] = hour % 10;
		mode_fnd[2] = min / 10;
		mode_fnd[3] = min % 10;
	}

	return;
}

/*************************
function name : control_mode2()
parameter : struct msgbuf
function content : If input received, control output data and send them when mode2
*************************/
void control_mode2(struct msgbuf msg)
{
	if(msg.sw_data[0] == 1) // switch mode
	{
		int num = mode_fnd[1] * mode2_flag * mode2_flag + mode_fnd[2] * mode2_flag + mode_fnd[3];

		if(mode2_flag == 10)
		{
			mode2_flag = 8;
		}
		else if(mode2_flag == 8)
		{
			mode2_flag = 4;
		}
		else if(mode2_flag == 4)
		{
			mode2_flag = 2;
		}
		else if(mode2_flag == 2)
		{
			mode2_flag = 10;
		}

		mode_fnd[3] = num % mode2_flag;
		num /= mode2_flag;
		mode_fnd[2] = num % mode2_flag;
		num /= mode2_flag;
		mode_fnd[1] = num % mode2_flag;	
	}
	else if(msg.sw_data[1] == 1) // push SW(2)
	{
		mode_fnd[1]++;
		if(mode_fnd[1] == mode2_flag)
		{
			mode_fnd[1] = 0;
		}
	}
	else if(msg.sw_data[2] == 1) // push SW(3)
	{
		mode_fnd[2]++;
		if(mode_fnd[2] == mode2_flag)
		{
			mode_fnd[2] = 0;
			mode_fnd[1]++;
			
			if(mode_fnd[1] == mode2_flag)
				mode_fnd[1] = 0;
		}
	}
	else if(msg.sw_data[3] == 1) // push SW(4)
	{
		mode_fnd[3]++;
		if(mode_fnd[3] == mode2_flag)
		{
			mode_fnd[3] = 0;
			mode_fnd[2]++;

			if(mode_fnd[2] == mode2_flag)
			{
				mode_fnd[2] = 0;
				mode_fnd[1]++;

				if(mode_fnd[1] == mode2_flag)
				{
					mode_fnd[1] = 0;
				}
			}
		}
	}

	return;
}

/*************************
function name : control_mode3()
parameter : struct msgbuf
function content : If input received, control output data and send them when mode3
*************************/
void control_mode3(struct msgbuf msg)
{
	int i;
	int Isnew = 0;
	unsigned char new_char;

	if(msg.sw_data[0] == 1) // push SW(1)
	{
		if(mode3_dot == 1)
		{
			if(mode3_prev_num != 1)
			{
				mode3_key_cnt = 0;
				Isnew = 1;
			}				

			new_char = mode3_sw[0][mode3_key_cnt++];

			if(mode3_key_cnt == 3)
				mode3_key_cnt = 0;

			mode3_prev_num = 1;
		}
		else
		{
			mode3_key_cnt = 0;
			new_char = '1';
			Isnew = 1;
		}			
	}
	else if(msg.sw_data[1] == 1 && msg.sw_data[2] == 1) // push SW(2) and SW(3)
	{
		for(i=0;i<mode3_len;i++)
			mode3_lcd[i]= ' ';

		mode3_len=0;		
	}
	else if(msg.sw_data[1] == 1) // push SW(2)
	{
		if(mode3_dot == 1)
		{
			if(mode3_prev_num != 2)
			{
				mode3_key_cnt = 0;
				Isnew = 1;
			}
				
			new_char = mode3_sw[1][mode3_key_cnt++];

			if(mode3_key_cnt == 3)
				mode3_key_cnt = 0;

			mode3_prev_num = 2;
		}
		else
		{
			mode3_key_cnt = 0;
			new_char = '2';
			Isnew = 1;
		}
	}
	else if(msg.sw_data[2] == 1) // push SW(3)
	{
		if(mode3_dot == 1)
		{
			if(mode3_prev_num != 3)
			{
				mode3_key_cnt = 0;
				Isnew = 1;
			}
			
			new_char = mode3_sw[2][mode3_key_cnt++];

			if(mode3_key_cnt == 3)
				mode3_key_cnt = 0;

			mode3_prev_num = 3;	
		}
		else
		{
			mode3_key_cnt = 0;
			new_char = '3';
			Isnew = 1;
		}
	}
	else if(msg.sw_data[3] == 1) // push SW(4)
	{
		if(mode3_dot == 1)
		{
			if(mode3_prev_num != 4)
			{
				mode3_key_cnt = 0;
				Isnew = 1;
			}				
			
			new_char = mode3_sw[3][mode3_key_cnt++];

			if(mode3_key_cnt == 3)
				mode3_key_cnt = 0;

			mode3_prev_num = 4;
		}
		else
		{
			mode3_key_cnt = 0;
			new_char = '4';
			Isnew = 1;
		}
	}
	else if(msg.sw_data[4] == 1 && msg.sw_data[5] == 1) // push SW(5) and SW(6)
	{
		Isnew = 2;
		mode3_dot = (mode3_dot==1)?2:1;
		mode3_key_cnt = 0;
		mode3_prev_num = 0;
	}
	else if(msg.sw_data[4] == 1) // push SW(5)
	{
		if(mode3_dot == 1)
		{
			if(mode3_prev_num != 5)
			{
				mode3_key_cnt = 0;
				Isnew = 1;
			}				
			
			new_char = mode3_sw[4][mode3_key_cnt++];

			if(mode3_key_cnt == 3)
				mode3_key_cnt = 0;

			mode3_prev_num = 5;
		}
		else
		{
			mode3_key_cnt = 0;
			new_char = '5';
			Isnew = 1;
		}
	}	
	else if(msg.sw_data[5] == 1) // push SW(6)
	{
		if(mode3_dot == 1)
		{
			if(mode3_prev_num != 6)
			{
				mode3_key_cnt = 0;
				Isnew = 1;
			}	
			
			new_char = mode3_sw[5][mode3_key_cnt++];

			if(mode3_key_cnt == 3)
				mode3_key_cnt = 0;

			mode3_prev_num = 6;
		}
		else
		{
			mode3_key_cnt = 0;
			new_char = '6';
			Isnew = 1;
		}
	}
	else if(msg.sw_data[6] == 1) // push SW(7)
	{
		if(mode3_dot == 1)
		{
			if(mode3_prev_num != 7)
			{
				mode3_key_cnt = 0;
				Isnew = 1;
			}	
			
			new_char = mode3_sw[6][mode3_key_cnt++];

			if(mode3_key_cnt == 3)
				mode3_key_cnt = 0;

			mode3_prev_num = 7;
		}
		else
		{
			mode3_key_cnt = 0;
			new_char = '7';
			Isnew = 1;
		}
	}
	else if(msg.sw_data[7] == 1 && msg.sw_data[8] == 1) // push SW(8) and SW(9)
	{
		Isnew = 1;
		mode3_key_cnt = 0;
		mode3_prev_num = 0;
		new_char = ' ';
	}
	else if(msg.sw_data[7] == 1) // push SW(8)
	{
		if(mode3_dot == 1)
		{
			if(mode3_prev_num != 8)
			{
				mode3_key_cnt = 0;
				Isnew = 1;
			}	
			
			new_char = mode3_sw[7][mode3_key_cnt++];

			if(mode3_key_cnt == 3)
				mode3_key_cnt = 0;

			mode3_prev_num = 8;
		}
		else
		{
			mode3_key_cnt = 0;
			new_char = '8';
			Isnew = 1;
		}
	}
	else if(msg.sw_data[8] == 1) // push SW(9)
	{
		if(mode3_dot == 1)
		{
			if(mode3_prev_num != 9)
			{
				mode3_key_cnt = 0;
				Isnew = 1;
			}	
			
			new_char = mode3_sw[8][mode3_key_cnt++];

			if(mode3_key_cnt == 3)
				mode3_key_cnt = 0;

			mode3_prev_num = 9;
		}
		else
		{
			mode3_key_cnt = 0;
			new_char = '9';
			Isnew = 1;
		}
	}
	if(Isnew == 1) // display new character 
	{
		if(mode3_len == 32) // lcd character is full
		{
			for(i=0;i<31;i++)
				mode3_lcd[i] = mode3_lcd[i+1];
			
			if(new_char == ' ')
				mode3_lcd[31] = ' ';
			else
				mode3_lcd[31] = new_char;
		}
		else
		{
			if(new_char == ' ')
				mode3_len++;
			else
				mode3_lcd[mode3_len++] = new_char;	
		}				
	}
	else if(Isnew == 0) // switch present character
		mode3_lcd[mode3_len-1] = new_char;

	mode3_count++;
	
	if(mode3_count == 10000)
		mode3_count = 0;

	return;
}

/*************************
function name : control_mode4()
parameter : struct msgbuf
function content : If input received, control output data and send them when mode4
*************************/
void control_mode4(struct msgbuf msg)
{
	int i;

	if(msg.sw_data[0] == 1) // initialize
	{
		for(i=0;i<10;i++)
			mode4_dot[i] = 0;
		
		mode4_y = 0;
		mode4_x = 0x40;
		mode4_cursor = 1;
	}
	else if(msg.sw_data[1] == 1) // move up
	{
		if(mode4_y != 0)
			mode4_y--;
	}
	else if(msg.sw_data[2] == 1) // on/off cursur mode
	{
		mode4_cursor = !mode4_cursor;
	}
	else if(msg.sw_data[3] == 1) // move left
	{
		if(mode4_x != 0x40)
			mode4_x = mode4_x << 1;
	}
	else if(msg.sw_data[4] == 1) // select
	{
		mode4_dot[mode4_y] = mode4_dot[mode4_y] ^ mode4_x;
	}
	else if(msg.sw_data[5] == 1) // move right
	{
		if(mode4_x != 1)
			mode4_x = mode4_x >> 1;
	}
	else if(msg.sw_data[6] == 1) // clear
	{
		for(i=0;i<10;i++)
			mode4_dot[i] = 0;
	}
	else if(msg.sw_data[7] == 1) // move down
	{
		if(mode4_y != 9)
			mode4_y++;
	}
	else if(msg.sw_data[8] == 1) // reverse
	{
		for(i=0;i<10;i++)
		{
			mode4_dot[i] = mode4_dot[i] ^ 0x7f;
		}
	}

	mode4_count++;

	if(mode4_count == 10000)
		mode4_count = 0;

	return;
}

/*************************
function name : control_mode5()
parameter : struct msgbuf
function content : If input received, control output data and send them when mode5
*************************/
void control_mode5(struct msgbuf msg)
{
	if(msg.sw_data[3] == 1) // move left
	{
		if(mode5_me != 0x40)
			mode5_me = mode5_me << 1;
	}
	else if(msg.sw_data[5] == 1) // move right
	{
		if(mode5_me != 1)
			mode5_me = mode5_me >> 1;
	}

	return;
}