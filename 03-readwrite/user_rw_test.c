#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>    // open
#include <unistd.h>   // close, read, write
#include <string.h>   // strlen

#define DEVICE_PATH "/dev/readwrite_demo"

int main() {
    int fd;
    char read_buf[1024] = {0};  // 读取缓冲区
    char write_buf[] = "Hello from Userspace!";  // 要写入的数据

    // 打开设备文件
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }

    // 先读取设备中已有的内容
    if (read(fd, read_buf, sizeof(read_buf)) < 0) {
        perror("read");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("Read from device (before write): %s\n", read_buf);

    // 写入数据到设备
    if (write(fd, write_buf, strlen(write_buf)) < 0) {
        perror("write");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("Written to device: %s\n", write_buf);

    // 再次读取设备内容
    lseek(fd, 0, SEEK_SET);  // 重新定位文件偏移量
    memset(read_buf, 0, sizeof(read_buf));  // 清空读取缓冲区

    if (read(fd, read_buf, sizeof(read_buf)) < 0) {
        perror("read");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("Read from device (after write): %s\n", read_buf);

    // 关闭设备
    close(fd);
    return EXIT_SUCCESS;
}
