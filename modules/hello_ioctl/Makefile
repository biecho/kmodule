obj-m := hello_module.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all: hello_module

hello_module:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module.symvers modules.order hello_user

install: all
	sudo insmod hello_module.ko

uninstall:
	sudo rmmod hello_module

hello_user: hello_user.c module_ioctl.h
	gcc -o hello_user hello_user.c

