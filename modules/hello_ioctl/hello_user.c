#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "module_ioctl.h"

#define NUM_PAGES 10  // Number of pages to allocate.

int main() {
    int fd, ret;
    int value = 0;
    char *allocated_memory;
    unsigned long virt_addr;

    fd = open("/dev/hello_device", O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device");
        return -1;
    }

    ret = ioctl(fd, IOCTL_GET_VALUE, &value);
    if (ret < 0) {
        perror("Failed to get value from kernel");
        close(fd);
        return -1;
    }

    printf("Received value: %d\n", value);

    allocated_memory = malloc(NUM_PAGES * 4096);
    if (!allocated_memory) {
        perror("Failed to allocate memory");
        close(fd);
        return -1;
    }

    // Touch each page to ensure it's populated in the page table.
    for (int i = 0; i < NUM_PAGES; i++) {
        allocated_memory[i * 4096] = i;  // Write a value to the start of each page.
    }

    virt_addr = (unsigned long)allocated_memory;
    printf("Allocated memory at virtual address: %lx\n", virt_addr);

    // Fetch the physical address corresponding to the allocated virtual address.
    ret = ioctl(fd, IOCTL_GET_PHYS_ADDR, &virt_addr);
    if (ret < 0) {
        perror("Failed to get physical address for the virtual address");
        free(allocated_memory);
        close(fd);
        return -1;
    }

    // virt_addr now contains the physical address.
    printf("Physical address corresponding to virtual address: %lx\n", virt_addr);

    free(allocated_memory);
    close(fd);
    return 0;
}

