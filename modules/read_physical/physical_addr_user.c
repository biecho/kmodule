#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h> 

// The ioctl command and structure must match the kernel module
#define IOCTL_GET_PHY_ADDR _IOR('k', 1, struct vaddr_paddr_conv)
struct vaddr_paddr_conv {
    unsigned long vaddr;
    unsigned long paddr;
};

unsigned long get_physical_address_from_pagemap(unsigned long vaddr) {
    int pagemap_fd;
    unsigned long paddr = 0;
    off_t offset;
    ssize_t bytes_read;
    const size_t pagemap_entry_size = sizeof(uint64_t);

    pagemap_fd = open("/proc/self/pagemap", O_RDONLY);
    if (pagemap_fd < 0) {
        perror("open pagemap");
        return 0;
    }

    // Calculate the offset for the virtual address
    offset = (vaddr / sysconf(_SC_PAGESIZE)) * pagemap_entry_size;

    if (lseek(pagemap_fd, offset, SEEK_SET) == (off_t) -1) {
        perror("lseek pagemap");
        close(pagemap_fd);
        return 0;
    }

    bytes_read = read(pagemap_fd, &paddr, pagemap_entry_size);
    if (bytes_read < 0) {
        perror("read pagemap");
        close(pagemap_fd);
        return 0;
    }

    close(pagemap_fd);

    // Shift the pagemap entry to get the physical address
    paddr = paddr & ((1ULL << 55) - 1); // Mask out the flag bits
    paddr = paddr * sysconf(_SC_PAGESIZE); // Get the physical address

    return paddr;
}


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

    unsigned long paddr_pagemap = get_physical_address_from_pagemap((unsigned long)buffer);
    printf("Physical Address (pagemap): %lx\n", paddr_pagemap);


    // Close the device file
    close(fd);
    free(buffer);

    return 0;
}
