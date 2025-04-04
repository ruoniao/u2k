obj-m += netlink_kernel.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -o netlink_user netlink_user.c
	insmod netlink_kernel.ko

clean:
	rmmod netlink_kernel
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f netlink_user
