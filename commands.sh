#!/bin/bash

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# For inside the VM
mount_host_files() {
    if mountpoint -q "/mnt/host_files"; then
        echo -e "${RED}Mount point /mnt/host_files already exists.${NC}"
        return 1
    fi

    mkdir -p /mnt/host_files
    mount -t 9p -o trans=virtio host0 /mnt/host_files && \
    echo -e "${GREEN}Host files mounted to /mnt/host_files${NC}" || \
    echo -e "${RED}Failed to mount host files.${NC}"
}

# For inside the VM when in kernel directory
prepare_modules() {
    if [ ! -f "Kconfig" ] || [ ! -f "Makefile" ]; then
        echo -e "${RED}Current directory doesn't seem to be a kernel directory.${NC}"
        return 1
    fi

    make modules_prepare && \
    echo -e "${GREEN}Modules prepared${NC}" || \
    echo -e "${RED}Failed to prepare modules.${NC}"
}

# For host to connect to VM
ssh_vm() {
    local IMAGE_PATH="$1"
    if [ -z "$IMAGE_PATH" ]; then
        echo -e "${RED}Please provide the path to the IMAGE as an argument.${NC}"
        return 1
    fi

    # Quick connection check
    nc -z localhost 10021 &>/dev/null
    if [ $? -ne 0 ]; then
        echo -e "${RED}Unable to connect to VM. Is it running and SSH configured correctly?${NC}"
        return 1
    fi

    ssh -i "$IMAGE_PATH/bullseye.id_rsa" -p 10021 -o "StrictHostKeyChecking no" root@localhost
}


