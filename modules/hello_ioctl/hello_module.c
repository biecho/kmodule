#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/mm.h>
#include "module_ioctl.h"


MODULE_LICENSE("GPL");

#define DEVICE_NAME "hello_device"
#define CLASS_NAME "hello"

static int major_number;
static struct class* hello_class = NULL;
static struct device* hello_device = NULL;

static int hello_open(struct inode *, struct file *);
static int hello_release(struct inode *, struct file *);
static long hello_ioctl(struct file *, unsigned int, unsigned long);

static struct file_operations hello_fops = {
    .open = hello_open,
    .release = hello_release,
    .unlocked_ioctl = hello_ioctl,
};

static int __init hello_init(void) {
    printk(KERN_INFO "Hello: Initializing the Hello Module\n");

    major_number = register_chrdev(0, DEVICE_NAME, &hello_fops);
    if (major_number < 0) {
        printk(KERN_ALERT "Hello failed to register a major number\n");
        return major_number;
    }

    hello_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(hello_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(hello_class);
    }

    hello_device = device_create(hello_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(hello_device)) {
        class_destroy(hello_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(hello_device);
    }

    return 0;
}

static void __exit hello_exit(void) {
    device_destroy(hello_class, MKDEV(major_number, 0));
    class_unregister(hello_class);
    class_destroy(hello_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "Hello: Goodbye from the Hello Module!\n");
}

static int hello_open(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "Hello: Device opened\n");
    return 0;
}

static int hello_release(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "Hello: Device closed\n");
    return 0;
}

static int translate_user_virt_to_phys(
  unsigned long virt_addr,
  unsigned long *phys_addr) {
    struct page *page;
    unsigned long offset;
    int ret;
    unsigned long flags = 0; // We just want to read the physical address

    printk(KERN_INFO "Translating virtual address: %lx\n", virt_addr);  // Print the input virtual address

    ret = get_user_pages(virt_addr, 1, flags, &page, NULL);
    if (ret != 1) {
        printk(KERN_ERR "Failed to retrieve page for virtual address: %lx\n", virt_addr);  // Log error if unable to get page
        return -EFAULT;
    }

    offset = virt_addr & ~PAGE_MASK;
    *phys_addr = (page_to_pfn(page) << PAGE_SHIFT) + offset;

    printk(KERN_INFO "Translated to physical address: %lx with offset: %lx\n", *phys_addr, offset);  // Print the resulting physical address and offset

    put_page(page);  // Release the page reference.
    return 0;
}

static long hello_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
    int ret;
    unsigned long phys_addr;
    int dummy_value = 12345;

    switch(cmd) {
        case IOCTL_WRITE_MSG:
            printk(KERN_INFO "Hello: Hello World from Kernel\n");
            break;
        case IOCTL_GET_VALUE:
            if (copy_to_user((int *)arg, &dummy_value, sizeof(int))) {
                // Error copying to user
                return -EFAULT;
            }
            break;
        case IOCTL_GET_PHYS_ADDR: {
            ret = translate_user_virt_to_phys(arg, &phys_addr);
            if (ret != 0) {
                return ret; 
            }

            if (copy_to_user((void __user *)arg, &phys_addr, sizeof(phys_addr))) {
                return -EFAULT; 
            }
            break;
        }

        default:
            return -EINVAL;
    }
    return 0;
}

module_init(hello_init);
module_exit(hello_exit);

