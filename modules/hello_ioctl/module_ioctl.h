#ifndef MODULE_IOCTL_H
#define MODULE_IOCTL_H

#ifdef __KERNEL__
    #include <linux/ioctl.h>
#else
    #include <sys/ioctl.h>
#endif

// IOCTL commands

// Command to write a message to the kernel. The message will be passed as a char pointer.
#define IOCTL_WRITE_MSG _IOW(0x90, 0, char *)

// Command to get a dummy value from the kernel. The kernel will return an int.
#define IOCTL_GET_VALUE _IOR(0x90, 1, int *)

// Command to translate a user-space virtual address to a physical address.
// The user-space application passes a virtual address (as an unsigned long) 
// to the kernel, and the kernel returns the corresponding physical address.
#define IOCTL_GET_PHYS_ADDR _IOWR(0x90, 2, unsigned long)

#endif // MODULE_IOCTL_H

