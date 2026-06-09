// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

static int __init hello_kernel_init(void)
{
    pr_info("Hello_kernel: Module loaded on Orange Pi CM4-RK3566!\n");
    return 0;
}

static void __exit hello_kernel_exit(void)
{
    pr_info("Hello_kernel: Module unloaded on Orange Pi CM4-RK3566!\n");
}

module_init(hello_kernel_init);
module_exit(hello_kernel_exit);

MODULE_AUTHOR("Tans");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("A simple hello world kernel module for Orange Pi CM4-RK3566");