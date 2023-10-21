#!/bin/bash

# Check if kernel source path is provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 /path/to/kernel/source"
    exit 1
fi

KERNEL_SRC_PATH="$1"

# Change to the directory of the kernel source
cd "$KERNEL_SRC_PATH" || { echo "Failed to change directory to $KERNEL_SRC_PATH"; exit 1; }

# Compile the KVM Modules
echo "Building KVM modules..."
make M=arch/x86/kvm/ || { echo "Build failed!"; exit 1; }

echo "KVM module build completed!"

