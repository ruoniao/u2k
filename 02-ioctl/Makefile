obj-m += my_ioctl_driver.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	rm -f *.ko *.o *.mod.o *.mod.c *.symvers *.order