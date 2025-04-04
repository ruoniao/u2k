#include <linux/module.h>    // 内核模块头文件
#include <linux/kernel.h>    // 内核日志
#include <linux/proc_fs.h>   // proc 文件系统
#include <linux/uaccess.h>   // 用户空间数据交互 API（copy_to_user, copy_from_user）
#include <linux/slab.h>      // kmalloc/kfree

#define PROC_FILENAME "procfs_demo"   // /proc 目录下的文件名
#define BUFFER_SIZE 1024              // 缓冲区大小

static struct proc_dir_entry *proc_file;  // /proc 文件指针
static char *kernel_buffer;  // 内核缓冲区
static int data_size = 0;  // 当前数据长度

// 读取 /proc/procfs_demo 文件的内容
static ssize_t proc_read(struct file *file, char __user *user_buf, size_t count, loff_t *pos) {
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

// 写入数据到 /proc/procfs_demo 文件
static ssize_t proc_write(struct file *file, const char __user *user_buf, size_t count, loff_t *pos) {
    if (count > BUFFER_SIZE) {
        count = BUFFER_SIZE;  // 限制最大写入数据
    }

    if (copy_from_user(kernel_buffer, user_buf, count) != 0) {
        return -EFAULT;  // 复制失败
    }

    data_size = count;
    kernel_buffer[data_size] = '\0';  // 确保字符串结尾
    return count;  // 返回写入的字节数
}

// 文件操作结构
static struct proc_ops proc_fops = {
    .proc_read = proc_read,
    .proc_write = proc_write,
};

// 初始化内核模块
static int __init procfs_demo_init(void) {
    // 分配内核缓冲区
    kernel_buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!kernel_buffer) {
        printk(KERN_ERR "Failed to allocate memory\n");
        return -ENOMEM;
    }

    // 在 /proc 下创建文件
    proc_file = proc_create(PROC_FILENAME, 0666, NULL, &proc_fops);
    if (!proc_file) {
        kfree(kernel_buffer);
        printk(KERN_ERR "Failed to create /proc/%s\n", PROC_FILENAME);
        return -ENOMEM;
    }

    // 初始化内核缓冲区内容
    snprintf(kernel_buffer, BUFFER_SIZE, "Hello from Kernel!");
    data_size = strlen(kernel_buffer);

    printk(KERN_INFO "/proc/%s created\n", PROC_FILENAME);
    return 0;
}

// 卸载内核模块
static void __exit procfs_demo_exit(void) {
    remove_proc_entry(PROC_FILENAME, NULL);  // 删除 /proc 下的文件
    kfree(kernel_buffer);  // 释放内核缓冲区
    printk(KERN_INFO "/proc/%s removed\n", PROC_FILENAME);
}

module_init(procfs_demo_init);
module_exit(procfs_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ruoniao");
MODULE_DESCRIPTION("ProcFS Kernel Read/Write Demo");
