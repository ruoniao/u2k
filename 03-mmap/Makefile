obj-m += mmap_driver.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	gcc -o user_mmap_test user_mmap_test.c

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f user_mmap_test

