#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include "module_ioctl.h"

#define DEVICE_NAME "hello_device"
#define CLASS_NAME "hello"

static int major_number;
static struct class* hello_class = NULL;
static struct device* hello_device = NULL;

static long hello_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);

static struct file_operations fops = {
    .unlocked_ioctl = hello_ioctl,
    .owner = THIS_MODULE,
};

static int __init hello_init(void) {
    printk(KERN_INFO "Hello: Initializing the Hello LKM\n");

    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "Hello failed to register a major number\n");
        return major_number;
    }
    printk(KERN_INFO "Hello: registered with major number %d\n", major_number);

    hello_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(hello_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(hello_class);
    }
    printk(KERN_INFO "Hello: device class registered\n");

    hello_device = device_create(hello_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(hello_device)) {
        class_destroy(hello_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(hello_device);
    }
    printk(KERN_INFO "Hello: device class created\n");

    return 0;
}

static void __exit hello_exit(void) {
    device_destroy(hello_class, MKDEV(major_number, 0));
    class_unregister(hello_class);
    class_destroy(hello_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "Hello: Goodbye from the LKM!\n");
}


// Function to translate user-space virtual address to physical address.
static int translate_user_virt_to_phys(unsigned long virt_addr, unsigned long *phys_addr) {
    struct page *page;
    unsigned long offset;
    int ret;
    unsigned long flags = 0;

    ret = get_user_pages(virt_addr, 1, flags, &page, NULL);
    if (ret != 1) {
        return -EFAULT;
    }

    offset = virt_addr & ~PAGE_MASK;
    *phys_addr = (page_to_pfn(page) << PAGE_SHIFT) + offset;

    put_page(page);  // Release the page reference.
    return 0;
}

static long hello_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
    int ret;
    int dummy_value = 12345;
    addr_translation_t addr_trans;

    switch (cmd) {
        case IOCTL_WRITE_MSG: {
            printk(KERN_INFO "Hello: Hello World from Kernel\n");
            break;
        }
        case IOCTL_GET_VALUE: {
            if (copy_to_user((int *)arg, &dummy_value, sizeof(int))) {
                return -EFAULT;
            }
            break;
        }
        case IOCTL_GET_PHYS_ADDR: {
            if (copy_from_user(&addr_trans, (addr_translation_t *)arg, sizeof(addr_translation_t))) {
                return -EFAULT;
            }
            ret = translate_user_virt_to_phys(addr_trans.virt_addr, &addr_trans.phys_addr);
            if (ret != 0) {
                return ret;
            }
            if (copy_to_user((addr_translation_t *)arg, &addr_trans, sizeof(addr_trans))) {
                return -EFAULT;
            }
            break;
        }
        default: {
            return -EINVAL;
        }
    }
    return 0;
}


module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Hello World module using IOCTLs");
MODULE_VERSION("0.1");

