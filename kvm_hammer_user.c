#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h> 
#include <time.h>
#include <emmintrin.h>  // For _mm_clflushopt and _mm_mfence

// The ioctl command and structure must match the kernel module
#define IOCTL_GET_PHY_ADDR _IOR('k', 1, struct vaddr_paddr_conv)
#define IOCTL_READ_HOST_PHY_ADDR _IOR('k', 2, struct host_paddr_data)

struct vaddr_paddr_conv {
    unsigned long vaddr;
    unsigned long paddr;
};

struct host_paddr_data {
    unsigned long host_paddr;
    unsigned long value;
};


unsigned long get_physical_address_from_pagemap(unsigned long vaddr) {
    int pagemap_fd;
    uint64_t paddr = 0;
    off_t offset;
    ssize_t bytes_read;
    const size_t pagemap_entry_size = sizeof(uint64_t);
    unsigned long page_offset = vaddr % sysconf(_SC_PAGESIZE);  // Calculate the offset within the page

    pagemap_fd = open("/proc/self/pagemap", O_RDONLY);
    if (pagemap_fd < 0) {
        perror("open pagemap");
        return 0;
    }

    // Calculate the offset for the virtual address in the pagemap file
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

    // Extract the physical page frame number and calculate the physical address
    paddr = paddr & ((1ULL << 55) - 1); // Mask out the flag bits
    paddr = paddr * sysconf(_SC_PAGESIZE); // Convert page frame number to physical address
    paddr += page_offset;  // Add the offset within the page

    return paddr;
}


int main() {
    int fd;
    struct vaddr_paddr_conv vp;
    struct host_paddr_data hp;
    char *buffer;
    const char test_value = 0xAA; // Test value to write and read

    // Allocate a buffer in user space
    buffer = malloc(1024);
    if (!buffer) {
        perror("malloc");
        return -1;
    }

    // Write a byte to the buffer
    *buffer = test_value;

    // Open the device file
    fd = open("/dev/kvmhammer", O_RDWR);
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

    // Set up and call the new IOCTL to read from the physical address
    hp.host_paddr = vp.paddr; // Use the physical address obtained
    if (ioctl(fd, IOCTL_READ_HOST_PHY_ADDR, &hp) == -1) {
        perror("ioctl read");
        close(fd);
        free(buffer);
        return -1;
    }

    printf("Read value: %lx\n", hp.value);

    // Compare only the relevant part (byte)
    if ((char)hp.value == test_value) {
        printf("Success: The values match.\n");
    } else {
        printf("Mismatch: The values do not match.\n");
    }

    // Timer variables
    struct timespec start, end;
    double elapsed_time;
    int count = 0;

    // Start the timer
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Loop until 64 ms have elapsed
    do {
        // Flush the cache line containing 'buffer'
        _mm_clflushopt(buffer);

        // Ensure all previous instructions have been executed
        _mm_mfence();

        if (ioctl(fd, IOCTL_READ_HOST_PHY_ADDR, &hp) == -1) {
            perror("ioctl read");
            // Handle error (e.g., break the loop or continue)
        }

        count++;
        clock_gettime(CLOCK_MONOTONIC, &end);

        elapsed_time = (end.tv_sec - start.tv_sec) * 1e3 + (end.tv_nsec - start.tv_nsec) / 1e6;  // Milliseconds
    } while (elapsed_time < 64.0);

    printf("Number of IOCTL calls in 64 ms: %d\n", count);

    return 0;

    // Close the device file and free the buffer
    close(fd);
    free(buffer);

    return 0;
}
