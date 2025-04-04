#include <linux/module.h>    // 内核模块头文件
#include <linux/kernel.h>    // 内核日志
#include <linux/fs.h>        // 文件操作
#include <linux/cdev.h>      // 字符设备
#include <linux/device.h>    // 设备类
#include <linux/uaccess.h>   // 用户空间数据交互 API（copy_to_user, copy_from_user）
#include <linux/slab.h>      // kmalloc/kfree

#define DEVICE_NAME "readwrite_demo"  // 设备名称
#define CLASS_NAME "rw_class"         // 设备类别
#define BUFFER_SIZE 1024              // 设备缓冲区大小

static dev_t dev_num;         // 设备号
static struct cdev rw_cdev;   // 字符设备
static struct class *rw_class;  // 设备类
static struct device *rw_device; // 设备指针
static char *kernel_buffer;  // 内核缓冲区
static int data_size = 0;  // 当前数据长度

// 设备读取操作：用户空间调用 read() 读取数据
static ssize_t rw_read(struct file *filp, char __user *user_buf, size_t count, loff_t *pos) {
    if (*pos >= data_size) {
        return 0;  // 没有更多数据可读
    }

    if (count > data_size - *pos) {
        count = data_size - *pos;  // 限制读取数据大小
    }

    if (copy_to_user(user_buf, kernel_buffer + *pos, count) != 0) {
        return -EFAULT;  // 复制失败
    }

    *pos += count;
    return count;  // 返回实际读取的字节数
}

// 设备写入操作：用户空间调用 write() 传输数据到内核
static ssize_t rw_write(struct file *filp, const char __user *user_buf, size_t count, loff_t *pos) {
    if (count > BUFFER_SIZE) {
        count = BUFFER_SIZE;  // 限制最大写入数据
    }

    if (copy_from_user(kernel_buffer, user_buf, count) != 0) {
        return -EFAULT;  // 复制失败
    }

    data_size = count;
    return count;  // 返回写入的字节数
}

// 设备文件操作
static struct file_operations fops = {
        .owner = THIS_MODULE,
        .read = rw_read,
        .write = rw_write,
};

// 初始化内核模块
static int __init rw_driver_init(void) {
    int ret;

    // 申请设备号
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ERR "Failed to allocate device number\n");
        return ret;
    }

    // 初始化字符设备
    cdev_init(&rw_cdev, &fops);
    ret = cdev_add(&rw_cdev, dev_num, 1);
    if (ret < 0) {
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ERR "Failed to add cdev\n");
        return ret;
    }

    // 创建设备类
    rw_class = class_create(CLASS_NAME);
    if (IS_ERR(rw_class)) {
        cdev_del(&rw_cdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(rw_class);
    }

    // 创建设备节点 /dev/readwrite_demo
    rw_device = device_create(rw_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(rw_device)) {
        class_destroy(rw_class);
        cdev_del(&rw_cdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(rw_device);
    }

    // 分配内核缓冲区
    kernel_buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!kernel_buffer) {
        device_destroy(rw_class, dev_num);
        class_destroy(rw_class);
        cdev_del(&rw_cdev);
        unregister_chrdev_region(dev_num, 1);
        return -ENOMEM;
    }

    // 内核初始化数据
    snprintf(kernel_buffer, BUFFER_SIZE, "Hello from Kernel!");
    data_size = strlen(kernel_buffer);

    printk(KERN_INFO "readwrite_demo driver initialized\n");
    return 0;
}

// 卸载内核模块
static void __exit rw_driver_exit(void) {
    kfree(kernel_buffer);
    device_destroy(rw_class, dev_num);
    class_destroy(rw_class);
    cdev_del(&rw_cdev);
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_INFO "readwrite_demo driver removed\n");
}

module_init(rw_driver_init);
module_exit(rw_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ruoniao");
MODULE_DESCRIPTION("Read/Write Kernel Driver with Initial Data");