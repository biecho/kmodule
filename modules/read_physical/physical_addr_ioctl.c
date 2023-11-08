#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/mm.h> // Required for get_user_pages
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/printk.h>
#include <linux/kvm_para.h> // Include if necessary for KVM hypercalls
#include <linux/device.h> // Required for device_create

#define DEVICE_NAME "mydevice"
#define IOCTL_GET_PHY_ADDR _IOR('k', 1, struct vaddr_paddr_conv)

struct vaddr_paddr_conv {
    unsigned long vaddr; // User space virtual address
    unsigned long paddr; // Physical address
};

static int major;
static struct class *class;
static struct cdev cdev;

static long get_physical_address(unsigned long vaddr, unsigned long *paddr) {
    struct page *page;
    long hypercall_ret;
    int ret;

    ret = get_user_pages(vaddr & PAGE_MASK, 1, FOLL_WRITE, &page, NULL);
    if (ret < 0) {
        pr_err("Failed to get user pages: %d\n", ret);
        return ret;
    }

    // Convert page to physical address
    *paddr = (page_to_pfn(page) << PAGE_SHIFT) | (vaddr & ~PAGE_MASK);

    pr_info("Virtual Address: %lx, Physical Address: %lx\n", vaddr, *paddr);

    // Release the page
    put_page(page);

    // Perform KVM hypercall with the physical address
    // Replace HYPERCALL_NUMBER with the appropriate number
    hypercall_ret = kvm_hypercall1(20, *paddr);

    // Log the return value of the hypercall
    pr_info("KVM hypercall returned: %ld\n", hypercall_ret);

    return 0; // Success
}

static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    struct vaddr_paddr_conv vp;
    long ret;

    switch (cmd) {
        case IOCTL_GET_PHY_ADDR:
            if (copy_from_user(&vp, (struct vaddr_paddr_conv *)arg, sizeof(vp))) {
                pr_err("Error copying from user\n");
                return -EFAULT;
            }

            ret = get_physical_address(vp.vaddr, &vp.paddr);
            if (ret) {
                pr_err("Error getting physical address: %ld\n", ret);
                return ret;
            }

            if (copy_to_user((struct vaddr_paddr_conv *)arg, &vp, sizeof(vp))) {
                pr_err("Error copying to user\n");
                return -EFAULT;
            }
            pr_info("IOCTL_GET_PHY_ADDR successful\n");
            break;

        default:
            pr_err("Unsupported IOCTL command\n");
            return -ENOTTY;
    }
    return 0;
}

static struct file_operations fops = {
    .unlocked_ioctl = device_ioctl,
    // other file operations
};

static int __init my_module_init(void) {
    dev_t dev_num;
    int err;

    // Allocate a major number dynamically
    err = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (err < 0) {
        pr_err("Failed to allocate a major number\n");
        return err;
    }
    major = MAJOR(dev_num);

    // Create device class
    class = class_create(THIS_MODULE, "mydevice_class");
    if (IS_ERR(class)) {
        unregister_chrdev_region(dev_num, 1);
        pr_err("Failed to create class\n");
        return PTR_ERR(class);
    }

    // Initialize and add cdev
    cdev_init(&cdev, &fops);
    err = cdev_add(&cdev, dev_num, 1);
    if (err) {
        class_destroy(class);
        unregister_chrdev_region(dev_num, 1);
        pr_err("Failed to add cdev\n");
        return err;
    }

    // Create device file in /dev
    if (IS_ERR(device_create(class, NULL, dev_num, NULL, DEVICE_NAME))) {
        cdev_del(&cdev);
        class_destroy(class);
        unregister_chrdev_region(dev_num, 1);
        pr_err("Failed to create device file\n");
        return -1;
    }

    pr_info("Module loaded successfully\n");
    return 0;
}

static void __exit my_module_exit(void) {
    dev_t dev_num = MKDEV(major, 0);

    // Remove device file and class
    device_destroy(class, dev_num);
    class_destroy(class);

    // Remove cdev and unregister device number
    cdev_del(&cdev);
    unregister_chrdev_region(dev_num, 1);

    pr_info("Module unloaded successfully\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple kernel module to provide physical addresses for user space virtual addresses");
