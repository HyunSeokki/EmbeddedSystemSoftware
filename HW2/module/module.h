#include <linux/ioctl.h>
#include "fpga_dot_font.h"

#define IOM_DEV_MAJOR 242
#define IOM_DEV_NAME "/dev/dev_driver"

//#define IOCTL_FND _IOW(IOM_DEV_MAJOR, 0, char *);
//#define IOCTL_LED _IOW(IOM_DEV_MAJOR, 1, char *);
//#define IOCTL_DOT _IOW(IOM_DEV_MAJOR, 2, char *);
//#define IOCTL_TEXT_LCD _IOW(IOM_DEV_MAJOR, 3, char *);