#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>    // open
#include <unistd.h>   // close, read, write
#include <string.h>   // strlen

#define PROC_PATH "/proc/procfs_demo"

int main() {
    int fd;
    char read_buf[1024] = {0};  // 读取缓冲区
    char write_buf[] = "Hello from Userspace!";  // 要写入的数据

    // 打开 /proc/procfs_demo 文件
    fd = open(PROC_PATH, O_RDWR);
    if (fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }

    // 先读取 /proc/procfs_demo 的内容
    if (read(fd, read_buf, sizeof(read_buf)) < 0) {
        perror("read");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("Read from /proc (before write): %s\n", read_buf);

    // 写入数据到 /proc/procfs_demo
    if (write(fd, write_buf, strlen(write_buf)) < 0) {
        perror("write");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("Written to /proc: %s\n", write_buf);

    // 再次读取 /proc/procfs_demo 内容
    lseek(fd, 0, SEEK_SET);  // 重新定位文件偏移量
    memset(read_buf, 0, sizeof(read_buf));  // 清空读取缓冲区

    if (read(fd, read_buf, sizeof(read_buf)) < 0) {
        perror("read");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("Read from /proc (after write): %s\n", read_buf);

    // 关闭文件
    close(fd);
    return EXIT_SUCCESS;
}
