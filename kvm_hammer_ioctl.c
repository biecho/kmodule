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
#include <linux/ktime.h>

#define DEVICE_NAME "kvmhammer"
#define IOCTL_GET_PHY_ADDR _IOR('k', 1, struct vaddr_paddr_conv)

struct vaddr_paddr_conv {
    unsigned long vaddr; // User space virtual address
    unsigned long paddr; // Physical address
};

#define IOCTL_READ_HOST_PHY_ADDR _IOR('k', 2, struct host_paddr_data)

struct host_paddr_data {
    unsigned long host_paddr; // Host physical address to read from
    unsigned long value;      // Data read from the address
};

#define IOCTL_READ_MULTI_HOST_PHY_ADDR _IOR('k', 3, struct multi_host_paddr_data)

struct multi_host_paddr_data {
    unsigned long *host_paddrs; // Pointer to an array of host physical addresses
    size_t count;               // Number of addresses in the array
};


static int major;
static struct class *class;
static struct cdev cdev;

static long get_physical_address(unsigned long vaddr, unsigned long *paddr) {
    struct page *page;
    unsigned long host_paddr;
    int ret;

    ret = get_user_pages(vaddr & PAGE_MASK, 1, FOLL_WRITE, &page, NULL);
    if (ret < 0) {
        pr_err("Failed to get user pages: %d\n", ret);
        return ret;
    }

    // Convert page to physical address
    *paddr = (page_to_pfn(page) << PAGE_SHIFT) | (vaddr & ~PAGE_MASK);

    // Release the page
    put_page(page);

    // Perform vmcall with the hypercall number 21 and guest physical address
    // Capture the host physical address returned by the vmcall
    asm volatile("vmcall"
                 : "=a"(host_paddr) // Capture the output in the host_paddr variable
                 : "a"(21), "b"(*paddr) // Hypercall number 21 and guest physical address in RBX
                 : "memory");

    // Update the paddr with the host physical address
    *paddr = host_paddr;

    return 0;
}

static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    struct vaddr_paddr_conv vp;
    struct host_paddr_data hp; // Declare hp variable
    long ret;
    unsigned long temp_value;  // Temporary variable for inline assembly
    unsigned long long start, end, duration;

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
            break;

        case IOCTL_READ_HOST_PHY_ADDR:
            if (copy_from_user(&hp, (struct host_paddr_data *)arg, sizeof(hp))) {
                pr_err("Error copying from user\n");
                return -EFAULT;
            }

            start = ktime_get_ns();

            asm volatile("vmcall"
             : "=a"(temp_value)  // Capture the output in EAX
             : "a"(22), "b"(hp.host_paddr)
             : "memory");

            end = ktime_get_ns();
            duration = end - start;

            hp.value = temp_value & 0xFF;  // Extracting only the relevant byte

            if (copy_to_user((struct host_paddr_data *)arg, &hp, sizeof(hp))) {
                pr_err("Error copying to user\n");
                return -EFAULT;
            }
            break;
        case IOCTL_READ_MULTI_HOST_PHY_ADDR: {
            struct multi_host_paddr_data mhp;
            unsigned long *user_paddrs;
            unsigned long i,j, rounds;
            unsigned long temp_value;

            if (copy_from_user(&mhp, (struct multi_host_paddr_data *)arg, sizeof(mhp))) {
                pr_err("Error copying from user\n");
                return -EFAULT;
            }

            // Allocate kernel memory for the array of addresses
            user_paddrs = kmalloc_array(mhp.count, sizeof(unsigned long), GFP_KERNEL);
            if (!user_paddrs) {
                return -ENOMEM;
            }

            // Copy the array of addresses from user space
            if (copy_from_user(user_paddrs, mhp.host_paddrs, mhp.count * sizeof(unsigned long))) {
                pr_err("Error copying address array from user\n");
                kfree(user_paddrs);
                return -EFAULT;
            }

            // Iterate over each address and perform the vmcall
            rounds = 1000000;
            start = ktime_get_ns();
            for (i = 0; i < rounds; i++) {
                mfence();

                for (j = 0; j < mhp.count; j++) {
                    asm volatile("vmcall"
                                : "=a"(temp_value)
                                : "a"(22), "b"(user_paddrs[j])
                                : "memory");

                }
            }
            end = ktime_get_ns();
            duration = end - start;

            pr_info("Total duration for IOCTL_READ_MULTI_HOST_PHY_ADDR: %llu ms\n", duration / 1000000);

            // Free the allocated kernel memory
            kfree(user_paddrs);
            break;
        }


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
MODULE_AUTHOR("Author");
MODULE_DESCRIPTION("KVM Hammer");
