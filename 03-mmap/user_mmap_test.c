#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define DEVICE_PATH "/dev/mmap_demo"
#define MEM_SIZE 4096

int main() {
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }

    // 申请内存映射
    char *mapped_mem = mmap(NULL, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped_mem == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("Kernel message: %s\n", mapped_mem);

    // 修改映射区域的内容
    snprintf(mapped_mem, MEM_SIZE, "Hello from Userspace!");

    // 解除映射
    munmap(mapped_mem, MEM_SIZE);
    close(fd);

    return EXIT_SUCCESS;
}
