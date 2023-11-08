obj-m += kvm_hammer_ioctl.o

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
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module.symvers modules.order kvm_hammer_ioctl

install: all
	sudo insmod kvm_hammer_ioctl.ko

uninstall:
	sudo rmmod kvm_hammer_ioctl

user: kvm_hammer_user.c
	gcc -o physical_addr_user kvm_hammer_user.c

