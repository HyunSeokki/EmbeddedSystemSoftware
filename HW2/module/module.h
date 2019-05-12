#include <linux/ioctl.h>
#include "fpga_dot_font.h"

#define IOM_DEV_MAJOR 242
#define IOM_DEV_NAME "/dev/dev_driver"

#define IOCTL_FPGA _IOW(242, 0, char*)