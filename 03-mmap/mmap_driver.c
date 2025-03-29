#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "mmap_demo"
#define CLASS_NAME "mmap_class"
#define MEM_SIZE PAGE_SIZE

static dev_t dev_num;
static struct cdev mmap_cdev;
static struct class *mmap_class;
static struct device *mmap_device;
static char *kernel_buffer;

// mmap 处理函数
static int mmap_driver_mmap(struct file *filp, struct vm_area_struct *vma) {
    unsigned long size = vma->vm_end - vma->vm_start;
    unsigned long pfn;

    if (size > MEM_SIZE) {
        return -EINVAL;
    }

    // 获取物理页帧号
    pfn = page_to_pfn(virt_to_page(kernel_buffer));

    // 映射物理地址到用户空间
    if (remap_pfn_range(vma, vma->vm_start, pfn, size, vma->vm_page_prot)) {
        return -EAGAIN;
    }

    return 0;
}

// 文件操作结构体
static struct file_operations fops = {
        .owner = THIS_MODULE,
        .mmap = mmap_driver_mmap,
};

// 模块初始化
static int __init mmap_driver_init(void) {
    int ret;

    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ERR "Failed to allocate device number\n");
        return ret;
    }

    cdev_init(&mmap_cdev, &fops);
    ret = cdev_add(&mmap_cdev, dev_num, 1);
    if (ret < 0) {
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ERR "Failed to add cdev\n");
        return ret;
    }

    mmap_class = class_create(CLASS_NAME);  // 修正 API 变更
    if (IS_ERR(mmap_class)) {
        cdev_del(&mmap_cdev);
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ERR "Failed to create class\n");
        return PTR_ERR(mmap_class);
    }

    mmap_device = device_create(mmap_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(mmap_device)) {
        class_destroy(mmap_class);
        cdev_del(&mmap_cdev);
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ERR "Failed to create device\n");
        return PTR_ERR(mmap_device);
    }

    kernel_buffer = kmalloc(MEM_SIZE, GFP_KERNEL);
    if (!kernel_buffer) {
        device_destroy(mmap_class, dev_num);
        class_destroy(mmap_class);
        cdev_del(&mmap_cdev);
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ERR "Failed to allocate kernel buffer\n");
        return -ENOMEM;
    }

    snprintf(kernel_buffer, MEM_SIZE, "Hello from Kernel!");

    printk(KERN_INFO "mmap_driver loaded successfully\n");
    return 0;
}

// 模块卸载
static void __exit mmap_driver_exit(void) {
    kfree(kernel_buffer);
    device_destroy(mmap_class, dev_num);
    class_destroy(mmap_class);
    cdev_del(&mmap_cdev);
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_INFO "mmap_driver unloaded\n");
}

module_init(mmap_driver_init);
module_exit(mmap_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple mmap kernel driver");
