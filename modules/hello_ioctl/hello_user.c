#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "module_ioctl.h"

#define NUM_PAGES 10

int main() {
    int fd, ret;
    int value = 0;
    char *allocated_memory;
    addr_translation_t addr_trans;

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

    addr_trans.virt_addr = (unsigned long)allocated_memory;
    printf("Allocated memory at virtual address: %lx\n", addr_trans.virt_addr);

    // Fetch the physical address corresponding to the allocated virtual address.
    ret = ioctl(fd, IOCTL_GET_PHYS_ADDR, &addr_trans);
    if (ret < 0) {
        perror("Failed to get physical address for the virtual address");
        free(allocated_memory);
        close(fd);
        return -1;
    }

    printf("Physical address corresponding to virtual address: %lx\n", addr_trans.phys_addr);

    free(allocated_memory);
    close(fd);
    return 0;
}

