Syscall Logging Kernel Module – via /proc

This kernel module demonstrates how a normal user-space system call (write) enters kernel space, performs operations, and returns back to user space.
We are using the /proc filesystem to receive input from user space.

What does this module do?

When the module loads → it prints:

Module loaded

When a user program writes to /proc/syslog_demo → the module prints:

entering kernel space (triggered by user write syscall)

performing a kernel operation (copy data from user to kernel)

returning back to user space

When the module unloads → it prints:

Module unloading

So we clearly see the flow of user → kernel → user.

How user actually calls kernel here?

User runs:

echo "hello" | sudo tee /proc/syslog_demo > /dev/null


This causes write() syscall from user.
The kernel routes that write to our custom .write handler in the module.

So this is a safe and legal method to observe syscall flow.

Files
File	Purpose
syslog_demo.c	the kernel module source
Makefile	to build the kernel module
Building & Running
make
sudo insmod syslog_demo.ko


Test by writing to /proc:

echo "test text" | sudo tee /proc/syslog_demo > /dev/null


Check logs:

dmesg | tail -n 20


Remove module:

sudo rmmod syslog_demo
dmesg | tail -n 20

Output Example (expected)
[syslog_demo] 1) module loaded
[syslog_demo] 2) entering kernel space from user
[syslog_demo] 3) performing kernel operation: got 10 bytes: "test text"
[syslog_demo] 4) returning from kernel to user space
[syslog_demo] 5) module unloading/exiting

Why /proc is used?

/proc is a virtual filesystem created by Linux kernel that allows user-space programs to communicate with kernel code using simple read/write system calls.

We did NOT directly modify the syscall table (dangerous and not allowed on new kernels).
Instead, we used /proc which is safe, recommended and standard.

User Space vs Kernel Space
Feature	User Space	Kernel Space
who runs here	normal apps (ls, echo, browsers)	kernel + drivers
access	restricted	full hardware access
crash impact	only crashes that process	can crash whole OS
communication	must call syscalls like read/write	receives those syscalls
our example	echo writes → syscalls write()	our module handles that write