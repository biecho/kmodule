#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <asm/io.h>

#define KVM_HC_READ_PHYS_ADDR 13
#define KVM_HC_GPA_TO_HPA 14 

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

    // Allocate memory
    test_memory = kmalloc(sizeof(u64), GFP_KERNEL);
    if(!test_memory) {
        printk(KERN_ERR "Failed to allocate memory.\n");
        return -ENOMEM;
    }

    // Write a pattern
    *test_memory = pattern;

    // Get the Guest Physical Address (GPA) of the memory
    gpa = virt_to_phys(test_memory);

    // Use a hypercall to get the Host Physical Address (HPA)
    asm volatile("vmcall"
                 : "=a"(hpa)
                 : "a"(KVM_HC_GPA_TO_HPA), "b"(gpa)
                 : "memory");

    // Use a second hypercall to read the value at that HPA and compare
    // (This is just to validate; you can remove it if unnecessary)
    // Note: This assumes you have a second hypercall to read an HPA
    asm volatile("vmcall"
                 : "=a"(read_pattern)
                 : "a"(KVM_HC_READ_PHYS_ADDR), "b"(hpa)
                 : "memory");

    if(read_pattern == pattern) {
        printk(KERN_INFO "Pattern matches!\n");
    } else {
        printk(KERN_ERR "Pattern does not match. Expected: %llx, Got: %llx\n", pattern, read_pattern);
    }

    kfree(test_memory);
    return 0;
}

static void __exit kvm_hypercall_tester_exit(void)
{
    printk(KERN_INFO "Exiting KVM hypercall tester module.\n");
}

module_init(kvm_hypercall_tester_init);
module_exit(kvm_hypercall_tester_exit);
