#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define DEVICE_PATH "/dev/mmap_demo" // 设备文件路径，对应内核模块注册的字符设备
#define MEM_SIZE 4096 // 映射的内存大小，与内核模块中的 MEM_SIZE 对应

int main() {
    // 打开字符设备文件，使用 O_RDWR 以读写模式访问
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }
    /*
     申请内存映射，将设备文件映射到用户空间，mmap 是一个系统调用。
     参数解析：
     - NULL：由内核决定映射的地址
     - MEM_SIZE：映射的大小
     - PROT_READ | PROT_WRITE：映射的内存可读可写
     - MAP_SHARED：映射是共享的，修改内容会同步到设备文件
     - fd：设备文件描述符
     - 0：偏移量，从文件起始位置映射
     */
    char *mapped_mem = mmap(NULL, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped_mem == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return EXIT_FAILURE;
    }

    // 读取并打印从内核映射到用户空间的内容
    printf("Kernel message: %s\n", mapped_mem);

    // 修改映射区域的内容，写入用户态的字符串
    snprintf(mapped_mem, MEM_SIZE, "Hello from Userspace!");

    // 解除映射
    munmap(mapped_mem, MEM_SIZE);
    close(fd);

    return EXIT_SUCCESS;
}
