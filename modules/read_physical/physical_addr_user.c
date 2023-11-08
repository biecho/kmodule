#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

// The ioctl command and structure must match the kernel module
#define IOCTL_GET_PHY_ADDR _IOR('k', 1, struct vaddr_paddr_conv)
struct vaddr_paddr_conv {
    unsigned long vaddr;
    unsigned long paddr;
};

int main() {
    int fd;
    struct vaddr_paddr_conv vp;
    char *buffer;

    // Allocate a buffer in user space
    buffer = malloc(1024);
    if (!buffer) {
        perror("malloc");
        return -1;
    }

    // Open the device file
    fd = open("/dev/mydevice", O_RDWR);
    if (fd < 0) {
        perror("open");
        free(buffer);
        return -1;
    }

    // Set the virtual address in the structure
    vp.vaddr = (unsigned long)buffer;

    // Call ioctl to get the physical address
    if (ioctl(fd, IOCTL_GET_PHY_ADDR, &vp) == -1) {
        perror("ioctl");
        close(fd);
        free(buffer);
        return -1;
    }

    printf("Virtual Address: %p, Physical Address: %lx\n", buffer, vp.paddr);

    // Close the device file
    close(fd);
    free(buffer);

    return 0;
}
