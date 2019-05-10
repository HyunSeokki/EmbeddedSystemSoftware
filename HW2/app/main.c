#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#define DEV_NAME "/dev/dev_driver"
#define IOCTL_FPGA _IOW(242, 0, char*)

int main(int argc, char *argv[])
{
	int i, ret, dev;
	char fnd_pos, fnd_data, time_interval, time_count;
	fnd_pos = fnd_data = time_interval = time_count = 0;

	if(argc != 4 || strlen(argv[3]) != 4)
	{
		printf("Please input the 3 parameters.\n");
		return -1;
	}

	time_interval = atoi(argv[1]);
	time_count = atoi(argv[2]);

	for(i = 0;i < strlen(argv[3]);i++)
	{
		if(argv[3][i] != '0')
		{
			fnd_pos = i;		
			fnd_data = argv[3][i] - '0';	
			break;
		}
	}
	
	dev = open(DEV_NAME, O_WRONLY);
	if(dev < 0)
	{
		printf("Device open error : %s\n", DEV_NAME);
		exit(1);
	}

	//ret = syscall(376, fnd_pos, fnd_data, time_interval, time_count);

	ioctl(dev, IOCTL_FPGA, 1);
	close(dev);
	return 0;
}