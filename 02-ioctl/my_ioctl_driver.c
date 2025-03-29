#include <linux/module.h>   // 模块宏和函数
#include <linux/kernel.h>   // 内核日志相关函数
#include <linux/fs.h>       // 文件系统操作
#include <linux/cdev.h>     // 字符设备结构
#include <linux/device.h>   // 设备创建
#include <linux/uaccess.h>  // 处理用户空间数据
#include <linux/version.h>  // 确保 LINUX_VERSION_CODE 可用

// 设备名称和类名
#define DEVICE_NAME "my_ioctl_dev"
#define CLASS_NAME "my_ioctl_class"

// IOCTL 命令定义
#define IOCTL_GET_VALUE _IOR('a', 1, int *) // 读取设备中的值
#define IOCTL_SET_VALUE _IOW('a', 2, int *) // 设置设备中的值

static dev_t dev_num;           // 设备号
static struct cdev my_cdev;     // 字符设备结构
static struct class *my_class;  // 设备类
static struct device *my_device; // 设备指针

/**
 * @brief 处理 IOCTL 命令
 *
 * @param file 指向打开的设备文件
 * @param cmd IOCTL 命令
 * @param arg 用户空间传递的参数
 * @return long 返回 0 表示成功，负数表示错误
 */
static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int value = 1234; // 设备中的内部值
    int user_value;

    switch (cmd) {
        case IOCTL_GET_VALUE:
            // `copy_to_user` 用于将数据从内核空间复制到用户空间
            /*
             * copy_to_user 介绍
                copy_to_user 是 Linux 内核中的一个 API，主要用于从内核空间向用户空间传输数据，确保数据安全性并防止非法访问。它的定义位于 linux/uaccess.h 头文件中：
                #include <linux/uaccess.h>
                long copy_to_user(void __user *to, const void *from, unsigned long n);
                如果内核直接访问用户空间地址，可能导致非法访问或内核崩溃。copy_to_user 提供了一种受控的方式来复制数据，同时进行权限检查。
             * */
            if (copy_to_user((int __user *)arg, &value, sizeof(value))) {
        return -EFAULT; // 复制失败，返回错误
    }
            printk(KERN_INFO "Sent value to user: %d\n", value);
            break;

        case IOCTL_SET_VALUE:
            // `copy_from_user` 用于从用户空间复制数据到内核空间
            if (copy_from_user(&user_value, (int __user *)arg, sizeof(user_value))) {
        return -EFAULT; // 复制失败，返回错误
    }
            printk(KERN_INFO "Received value from user: %d\n", user_value);
            break;

        default:
            return -EINVAL; // 无效命令
    }

    return 0; // 成功返回
}

// 定义文件操作结构体
static struct file_operations fops = {
        .owner = THIS_MODULE,   // 设备归属当前模块
        .unlocked_ioctl = my_ioctl, // 处理 IOCTL 调用
};

/**
 * @brief 模块初始化函数
 *
 * @return int 0 表示成功，负数表示失败
 */
static int __init my_init(void) {
    int ret;

    /*  分配字符设备号 (动态分配主设备号) 通讯 */
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ERR "Failed to allocate device number\n");
        return ret;
    }

    // 初始化字符设备
    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;

    // 将字符设备添加到系统中
    ret = cdev_add(&my_cdev, dev_num, 1);
    if (ret < 0) {
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ERR "Failed to add cdev\n");
        return ret;
    }

    // 创建设备类，供 `sysfs` 使用
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,4,0)
    my_class = class_create(CLASS_NAME); // Linux 6.4+ 版本
#else
    my_class = class_create(THIS_MODULE, CLASS_NAME); // 旧版本
#endif
    if (IS_ERR(my_class)) {
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ERR "Failed to create class\n");
        return PTR_ERR(my_class);
    }

    // 创建设备节点 `/dev/my_ioctl_dev`
    my_device = device_create(my_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(my_device)) {
        class_destroy(my_class);
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ERR "Failed to create device\n");
        return PTR_ERR(my_device);
    }

    printk(KERN_INFO "my_ioctl_driver loaded successfully\n");
    return 0;
}

/**
 * @brief 模块卸载函数
 */
static void __exit my_exit(void) {
    device_destroy(my_class, dev_num); // 移除设备文件
    class_destroy(my_class);           // 销毁设备类
    cdev_del(&my_cdev);                // 移除 cdev
    unregister_chrdev_region(dev_num, 1); // 释放设备号
    printk(KERN_INFO "my_ioctl_driver unloaded\n");
}

// 注册模块初始化和退出函数
module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ruoniao");
MODULE_DESCRIPTION("My IOCTL Driver");
