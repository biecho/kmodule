#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 /path/to/kernel/source"
    exit 1
fi

KERNEL_SRC_PATH="$1"
cd "$KERNEL_SRC_PATH" || { echo "Failed to change directory to $KERNEL_SRC_PATH"; exit 1; }

echo "Copying the configuration of the currently running kernel..."
cp /boot/config-$(uname -r) .config || { echo "Failed to copy the current kernel configuration!"; exit 1; }

echo "Updating the configuration using olddefconfig..."
make olddefconfig || { echo "Failed at make olddefconfig!"; exit 1; }

echo "Compiling the kernel..."
make -j$(nproc) || { echo "Kernel compilation failed!"; exit 1; }

echo "Kernel compilation completed!"

