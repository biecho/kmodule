#ifndef MODULE_IOCTL_H
#define MODULE_IOCTL_H

#ifdef __KERNEL__
    #include <linux/ioctl.h>
#else
    #include <sys/ioctl.h>
#endif

// IOCTL commands
#define IOCTL_WRITE_MSG _IOW(0x90, 0, char *)
#define IOCTL_GET_VALUE _IOR(0x90, 1, int *)

#endif // MODULE_IOCTL_H

