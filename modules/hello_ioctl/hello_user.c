#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "module_ioctl.h"

int main() {
    int fd, ret;
    int value = 0;

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

    close(fd);
    return 0;
}
