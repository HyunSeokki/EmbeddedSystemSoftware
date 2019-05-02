#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/input.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <termios.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

struct msgbuf
{
	long msgtype;
	int code;
	unsigned char sw_data[9];
	unsigned char fnd_data[4];
	unsigned char led_data;
	unsigned char lcd_data[32];
	unsigned char dot_data[10];
};

key_t key = 7273;
time_t current;
struct tm *t;