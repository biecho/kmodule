obj-m += kvm_hypercall_tester.o

# Default kernel directory using uname
KDIR := /lib/modules/$(shell uname -r)/build

# Check if the custom kernel directory exists and override KDIR if it does
ifneq ($(wildcard /mnt/host_files/linux/*),)
    KDIR := /mnt/host_files/linux
endif

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean: 
	$(MAKE) -C $(KDIR) M=$(PWD) clean

