// syslog_demo.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h> //copy_from_user & copy_to_user

#define PROC_NAME "syslog_demo"
#define BUF_SZ 128

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nihad");
MODULE_DESCRIPTION("Logs a user->kernel->user syscall path via /proc");

// proc write handler: runs in kernel due to user-space write()
static ssize_t demo_write(struct file *file, const char __user *ubuf, // ubuf → pointer to data in userspace
                          size_t len, loff_t *ppos)
{ // this function is called when user writes to /proc/syslog_demo

    char kbuf[BUF_SZ]; // allocate a buffer inside kernel to store data coming from user.

    // 2) entering kernel space (user issued write())
    printk(KERN_INFO "[syslog_demo] 2) entering kernel space from user\n");

    // 3) performing kernel operation (copy_from_user + trivial work)
    if (len >= BUF_SZ)
        len = BUF_SZ - 1;

    if (copy_from_user(kbuf, ubuf, len)) // safely copy data from user memory to kernel memory.//intead of memcpy(kbuf, ubuf, len)
        return -EFAULT;

    kbuf[len] = '\0';

    printk(KERN_INFO "[syslog_demo] 3) performing kernel operation: got %zu bytes: \"%s\"\n",
           len, kbuf);

    // 4) returning from kernel (completing write() syscall)
    printk(KERN_INFO "[syslog_demo] 4) returning from kernel to user space\n");

    // report all bytes consumed so user-space write() sees success
    return len;
}

// next is
// linking our write handler function to this /proc file.
// Whenever someone writes to /proc/syslog_demo → demo_write() gets called.
static const struct proc_ops demo_ops = {
    .proc_write = demo_write,
};

static int __init demo_init(void)
{
    // 1) module loaded
    printk(KERN_INFO "[syslog_demo] 1) module loaded\n"); // printed when module is inserted using insmod

    if (!proc_create(PROC_NAME, 0666, NULL, &demo_ops)) // creates a file /proc/syslog_demo
    {
        printk(KERN_ERR "[syslog_demo] failed to create /proc/%s\n", PROC_NAME);
        return -ENOMEM;
    }
    return 0;
}

static void __exit demo_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
    // 5) module unloading/exiting
    printk(KERN_INFO "[syslog_demo] 5) module unloading/exiting\n");
}

module_init(demo_init);
module_exit(demo_exit);

