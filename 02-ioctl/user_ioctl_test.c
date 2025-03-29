#include <stdio.h>      // 标准输入输出库
#include <stdlib.h>     // 标准库，包括 EXIT_SUCCESS 和 EXIT_FAILURE
#include <fcntl.h>      // 文件控制（open()）
#include <unistd.h>     // UNIX 标准头文件，包含 close()
#include <sys/ioctl.h>  // ioctl 相关的系统调用

// 设备文件路径，用户态程序通过它访问内核驱动
#define DEVICE_PATH "/dev/my_ioctl_dev"

// 定义 IOCTL 命令
#define IOCTL_GET_VALUE _IOR('a', 1, int *) // 读取设备中的整数值
#define IOCTL_SET_VALUE _IOW('a', 2, int *) // 设置设备中的整数值

int main() {
    // 打开设备文件，O_RDWR 允许读写
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("open"); // 如果打开失败，打印错误信息
        return EXIT_FAILURE;
    }

    // 要写入设备的值
    int value = 100;
    printf("Setting value to %d\n", value);

    // 通过 IOCTL 命令将值传递到内核驱动
    if (ioctl(fd, IOCTL_SET_VALUE, &value) < 0) {
        perror("ioctl set"); // 失败时打印错误
        close(fd);
        return EXIT_FAILURE;
    }

    // 读取设备存储的值
    value = 0; // 先清零，确保 ioctl 读取的是设备的值
    if (ioctl(fd, IOCTL_GET_VALUE, &value) < 0) {
        perror("ioctl get");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("Retrieved value: %d\n", value); // 打印从设备读取的值

    // 关闭设备文件
    close(fd);
    return EXIT_SUCCESS;
}
