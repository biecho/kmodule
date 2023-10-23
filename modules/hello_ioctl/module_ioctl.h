#ifndef MODULE_IOCTL_H
#define MODULE_IOCTL_H

#ifdef __KERNEL__
    #include <linux/ioctl.h>
#else
    #include <sys/ioctl.h>
#endif

// Define a structure to facilitate the translation of virtual to physical addresses.
typedef struct {
    unsigned long virt_addr;  // Input: Virtual address from user-space
    unsigned long phys_addr;  // Output: Translated physical address from kernel-space
} addr_translation_t;

// IOCTL commands

// Command to write a message to the kernel. The message will be passed as a char pointer.
#define IOCTL_WRITE_MSG _IOW(0x90, 0, char *)

// Command to get a dummy value from the kernel. The kernel will return an int.
#define IOCTL_GET_VALUE _IOR(0x90, 1, int *)

// Command to translate a user-space virtual address to a physical address.
// The user-space application passes an addr_translation_t structure to the kernel,
// providing a virtual address. The kernel fills in the corresponding physical address.
#define IOCTL_GET_PHYS_ADDR _IOWR(0x90, 2, addr_translation_t)

#endif // MODULE_IOCTL_H

