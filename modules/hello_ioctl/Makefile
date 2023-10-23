obj-m := hello_module.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module.symvers

install: all
	sudo insmod hello_module.ko

uninstall:
	sudo rmmod hello_module

hello_user:
	gcc -o hello_user hello_user.c


