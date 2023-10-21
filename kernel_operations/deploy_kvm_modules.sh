#!/bin/bash

# Ensure the script is run as root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 
   exit 1
fi

# Check if kernel source path is provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 /path/to/kernel/source"
    exit 1
fi

KERNEL_SRC_PATH="$1"

# Change to the directory of the kernel source
cd "$KERNEL_SRC_PATH" || { echo "Failed to change directory to $KERNEL_SRC_PATH"; exit 1; }

# Unload the Current Modules
echo "Unloading current KVM modules..."
modprobe -r kvm_intel kvm || { echo "Failed to unload some KVM modules! Ensure no VMs are running."; exit 1; }

# Backup the Current Modules
echo "Backing up current KVM modules..."
cp /lib/modules/$(uname -r)/kernel/arch/x86/kvm/kvm-intel.ko /lib/modules/$(uname -r)/kernel/arch/x86/kvm/kvm-intel.ko.backup
cp /lib/modules/$(uname -r)/kernel/arch/x86/kvm/kvm.ko /lib/modules/$(uname -r)/kernel/arch/x86/kvm/kvm.ko.backup

# Install the New Modules
echo "Installing new KVM modules..."
cp arch/x86/kvm/kvm-intel.ko /lib/modules/$(uname -r)/kernel/arch/x86/kvm/
cp arch/x86/kvm/kvm.ko /lib/modules/$(uname -r)/kernel/arch/x86/kvm/

# Load the New Modules
echo "Loading new KVM modules..."
modprobe kvm || { echo "Failed to load kvm module!"; exit 1; }
modprobe kvm_intel || { echo "Failed to load kvm_intel module!"; exit 1; }

echo "KVM module deployment for Intel completed!"

