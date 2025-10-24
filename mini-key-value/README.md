# Multi-Client Key-Value Store (Unix Domain Sockets)

## Overview
This project implements a multi-client, in-memory Key-Value Store (KV Store) using Unix domain sockets and POSIX threads. It supports basic commands like **SET**, **GET**, and **EXIT**, and can handle multiple clients concurrently.

---

## Features
- Concurrent client connections using `pthread_create()`
- Thread-safe in-memory key-value database
- Unix domain socket communication (`AF_UNIX`)
- Simple command interface: `SET / GET / EXIT`
- Graceful handling of multiple clients

---

## File Structure

| File       | Description                           |
|------------|---------------------------------------|
| `server.c` | KV Store server implementation        |
| `client.c` | Interactive client application        |
| `README.md`| Documentation and performance analysis|

---

## How to Build and Run

### Compile
```bash
gcc server.c -o server -lpthread
gcc client.c -o client

```

## Run the Server:
```bash
./server
```

## Run the Client (in another terminal):
```bash
./client
```

# System Call Analysis (strace)
Use 
```bash
* strace -c ./server and strace -c ./client to profile syscalls.
```
If your program makes thousands of system calls (read, write, open, connect), performance will degrade due to user<--->kernel transitions.

## When Syscalls Become a Bottleneck
Each syscall causes a context switch, CPU cache flush, and latency increase.
Example issues:
* read() / write(): 1000+ calls
* open() / close(): 500 calls
* connect(): 200 calls

## Optimization Strategies

| Problem                 | Cause                          | Fix                                 |
|-------------------------|--------------------------------|------------------------------------|
| Too many read/write      | Small unbuffered I/O           | Use buffered or batched I/O        |
| Too many open/close      | Repeated file access           | Cache and reuse file descriptors   |
| Too many connect         | Short-lived sessions           | Use persistent connections         |
| Too many pthread_create  | One thread per client          | Use a thread pool or epoll         |
| High latency             | Frequent user ↔ kernel transitions | Minimize syscall count          |

# Optimization Techniques
* Buffer I/O: Use fdopen() and standard I/O functions.
* Thread Pool: Fixed worker threads handle multiple clients.
* Persistent Connections: Keep client connections alive.
* Mutex-Protected Store: Use pthread_mutex for safe access.
* System Tuning: Increase backlog and buffer sizes with sysctl and setsockopt().
* Performance Measurement Tools
   - strace -T — Trace syscall timing
   - perf stat ./server — CPU and syscall stats
   - htop -t — View per-thread load
   - ltrace — Trace library calls
   - valgrind --tool=callgrind — Function-level profiling

## Summary

| Layer       | Problem               | Solution                  |
|------------|----------------------|---------------------------|
| User space | Too many I/O calls    | Buffer reads/writes       |
| Threading  | Thread per client     | Thread pool or epoll      |
| File access| Repeated open/close   | Cache FDs                 |
| Networking | Frequent connects     | Persistent sockets        |
| System     | Context switch cost   | Batch syscalls            |






