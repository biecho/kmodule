#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <asm/io.h>

#define KVM_HC_READ_PHYS_ADDR 13
#define KVM_HC_GPA_TO_HPA 14
#define KVM_EINVAL -22  // Assuming the error code is -22, adjust as necessary

MODULE_LICENSE("GPL");
MODULE_AUTHOR("biecho");
MODULE_DESCRIPTION("Kernel module for KVM hypercall testing.");
MODULE_VERSION("0.1");

static int __init kvm_hypercall_tester_init(void)
{
    u64 *test_memory;
    u64 pattern = 0xDEADBEEFDEADBEEF;
    u64 read_pattern;
    u64 gpa, hpa;

    printk(KERN_INFO "Initializing KVM hypercall tester module.\n");

    // Allocate memory
    test_memory = kmalloc(sizeof(u64), GFP_KERNEL);
    if(!test_memory) {
        printk(KERN_ERR "Failed to allocate memory.\n");
        return -ENOMEM;
    }
    printk(KERN_INFO "Memory allocated successfully.\n");

    // Write a pattern
    *test_memory = pattern;
    printk(KERN_INFO "Pattern %llx written to memory.\n", pattern);

    // Get the Guest Physical Address (GPA) of the memory
    gpa = virt_to_phys(test_memory);
    printk(KERN_INFO "GPA of the test memory: %llx\n", gpa);

    // Use a hypercall to get the Host Physical Address (HPA)
    asm volatile("vmcall"
                 : "=a"(hpa)
                 : "a"(KVM_HC_GPA_TO_HPA), "b"(gpa)
                 : "memory");

    if(hpa == KVM_EINVAL) {
        printk(KERN_ERR "Hypercall error: Invalid address or other error.\n");
        kfree(test_memory);  // Free memory before returning
        return -EINVAL;      // Signal an error
    } else {
        printk(KERN_INFO "HPA retrieved via hypercall: %llx\n", hpa);
    }

    // Use a second hypercall to read the value at that HPA and compare
    asm volatile("vmcall"
                 : "=a"(read_pattern)
                 : "a"(KVM_HC_READ_PHYS_ADDR), "b"(hpa)
                 : "memory");
    printk(KERN_INFO "Value read from HPA: %llx\n", read_pattern);

    if(read_pattern == pattern) {
        printk(KERN_INFO "Pattern matches!\n");
    } else {
        printk(KERN_ERR "Pattern does not match. Expected: %llx, Got: %llx\n", pattern, read_pattern);
    }

    kfree(test_memory);
    printk(KERN_INFO "Memory freed successfully.\n");
    return 0;
}

static void __exit kvm_hypercall_tester_exit(void)
{
    printk(KERN_INFO "Exiting KVM hypercall tester module.\n");
}

module_init(kvm_hypercall_tester_init);
module_exit(kvm_hypercall_tester_exit);

