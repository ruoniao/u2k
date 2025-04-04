#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "mmap_demo"   // 设备名称
#define CLASS_NAME "mmap_class"   // 设备类名称
#define MEM_SIZE PAGE_SIZE        // 分配一页内存的大小

static dev_t dev_num;             // 设备号
static struct cdev mmap_cdev;     // 字符设备结构体
static struct class *mmap_class;  // 设备类
static struct device *mmap_device; // 设备结构体
static char *kernel_buffer;       // 设备映射的内核缓冲区

// mmap 处理函数
static int mmap_driver_mmap(struct file *filp, struct vm_area_struct *vma) {
    unsigned long size = vma->vm_end - vma->vm_start; // 计算映射的大小
    unsigned long pfn;

    // 确保用户映射的大小不超过内核分配的缓冲区大小
    if (size > MEM_SIZE) {
        return -EINVAL; // 返回无效参数错误
    }

    // 获取 kernel_buffer 对应的物理页帧号
    pfn = page_to_pfn(virt_to_page(kernel_buffer));

    // 将物理地址映射到用户空间
    if (remap_pfn_range(vma, vma->vm_start, pfn, size, vma->vm_page_prot)) {
        return -EAGAIN; // 映射失败，返回重试错误
    }

    return 0; // 映射成功
}

// 文件操作结构体，定义 mmap 设备的操作
static struct file_operations fops = {
        .owner = THIS_MODULE,
        .mmap = mmap_driver_mmap, // 绑定 mmap 处理函数
};

// 模块初始化
static int __init mmap_driver_init(void) {
    int ret;

    // 分配设备号 (主设备号, 次设备号)
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ERR "Failed to allocate device number\n");
        return ret;
    }

    // 初始化字符设备结构体并添加到系统中
    cdev_init(&mmap_cdev, &fops);
    ret = cdev_add(&mmap_cdev, dev_num, 1);
    if (ret < 0) {
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ERR "Failed to add cdev\n");
        return ret;
    }

    // 创建设备类 (用于在 /sys/class/ 下创建设备)
    mmap_class = class_create(CLASS_NAME);
    if (IS_ERR(mmap_class)) {
        cdev_del(&mmap_cdev);
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ERR "Failed to create class\n");
        return PTR_ERR(mmap_class);
    }

    // 创建设备节点 (出现在 /dev/mmap_demo)
    mmap_device = device_create(mmap_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(mmap_device)) {
        class_destroy(mmap_class);
        cdev_del(&mmap_cdev);
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ERR "Failed to create device\n");
        return PTR_ERR(mmap_device);
    }

    // 分配内核缓冲区，用于 mmap 映射到用户空间
    kernel_buffer = kmalloc(MEM_SIZE, GFP_KERNEL);
    if (!kernel_buffer) {
        device_destroy(mmap_class, dev_num);
        class_destroy(mmap_class);
        cdev_del(&mmap_cdev);
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ERR "Failed to allocate kernel buffer\n");
        return -ENOMEM;
    }

    // 预填充一些数据，用户 mmap 后可以看到这个数据
    snprintf(kernel_buffer, MEM_SIZE, "Hello from Kernel!");

    printk(KERN_INFO "mmap_driver loaded successfully\n");
    return 0;
}

// 模块卸载
static void __exit mmap_driver_exit(void) {
    kfree(kernel_buffer);               // 释放内核缓冲区
    device_destroy(mmap_class, dev_num); // 销毁设备节点
    class_destroy(mmap_class);           // 销毁设备类
    cdev_del(&mmap_cdev);                // 删除字符设备
    unregister_chrdev_region(dev_num, 1); // 释放设备号
    printk(KERN_INFO "mmap_driver unloaded\n");
}

module_init(mmap_driver_init);
module_exit(mmap_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ruoniao");
MODULE_DESCRIPTION("Simple mmap kernel driver");
