#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define DEV_NAME "/dev/stopwatch"

int main(void)
{
	int fd;
	char buf[2]={0};
	
	fd = open(DEV_NAME, O_RDWR);
	if(fd < 0)
	{
		printf("Device open error : %s\n", DEV_NAME);
		exit(1);
	}
	
	write(fd, buf, 2);
	close(fd);
	
	return 0;
}