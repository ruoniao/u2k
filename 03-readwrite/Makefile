# 目标模块名
obj-m := read_write_driver.o
CC=gcc-12
# 内核编译路径
KDIR := /lib/modules/$(shell uname -r)/build

# 默认目标：编译内核模块
all:
	$(MAKE) -C $(KDIR) M=$(shell pwd) modules
	gcc user_rw_test.c -o user_rw_test
# 清理目标
clean:
	$(MAKE) -C $(KDIR) M=$(shell pwd) clean
	rm -f user_rw_test
