# Syscall Logging Kernel Module – via /proc

This kernel module demonstrates how a normal user-space system call (write) enters kernel space, performs operations, and returns back to user space.
We are using the /proc filesystem to receive input from user space.

What does this module do?

1- When the module loads → it prints:
``` C
Module loaded
```
2- When a user program writes to /proc/syslog_demo → the module prints:
```C
entering kernel space (triggered by user write syscall)
```
3- When doing some operation
```C
performing a kernel operation (copy data from user to kernel)
```
4- Returning
```C
returning back to user space
```
5- When the module unloads → it prints:
``` C
Module unloading
```
- So we clearly see the flow of user → kernel → user.

# How user actually calls kernel here?

User runs:
``` C

echo "hello" | sudo tee /proc/syslog_demo > /dev/null
```

This causes write() syscall from user.
The kernel routes that write to our custom .write handler in the module.
So this is a safe and legal method to observe syscall flow.

# Files
File	Purpose
syslog_demo.c	the kernel module source
Makefile	to build the kernel module
Building & Running
make
```C
sudo insmod syslog_demo.ko
```

Test by writing to /proc:
```C
echo "test text" | sudo tee /proc/syslog_demo > /dev/null
```

Check logs:

```C
dmesg | tail 
```


Remove module:
```C
sudo rmmod syslog_demo
dmesg | tail 
```


# Why /proc is used?

/proc is a virtual filesystem created by Linux kernel that allows user-space programs to communicate with kernel code using simple read/write system calls.

We did NOT directly modify the syscall table (dangerous and not allowed on new kernels).
Instead, we used /proc which is safe, recommended and standard.

