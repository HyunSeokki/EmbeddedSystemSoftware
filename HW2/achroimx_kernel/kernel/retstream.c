#include <linux/kernel.h>

asmlinkage int sys_retstream(char fnd_pos, char fnd_data, char time_interval, char time_count) {

	return ((fnd_pos << 24) | (fnd_data << 16) | (time_interval << 8) | time_count);
}
