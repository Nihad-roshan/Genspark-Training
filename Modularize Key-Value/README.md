# Modularized Key-Value Store using Dynamic Library (`dlopen()`)

This project modularizes the key-value store by moving its core logic (`kv_get`, `kv_set`) into a **shared library** that the **server** loads at runtime using the **dynamic linking functions** `dlopen()`, `dlsym()`, and `dlclose()`.

---

## Files Overview

| File | Description |
|------|--------------|
| `kvstore_functions.c` | Contains the actual key-value store logic (SET/GET handling). Compiled into a shared library `.so`. |
| `kvstore.h` | Header file declaring `kv_get()` and `kv_set()` for use by the server. |
| `server_main.c` | Server program that loads `libkvstore.so` dynamically at runtime using `dlopen()`/`dlsym()`. |
| `client.c` | client that connects to the server and sends commands. |
| `README.md` | This documentation. |

---

## Step-by-Step Commands (Terminal)

### 1. Create the Shared Library

Compile `kvstore.c` as a **position-independent shared library**:

```bash
gcc -fPIC -c kvstore_functions.c kvstore_functions.o
gcc -shared -o libkvstore.so kvstore_functions.o

```
Explanation:

- fPIC → Generate position-independent code (required for shared libs).

- c → Compile only, do not link yet.

- shared → Create a .so (shared object).

Output file: libkvstore.so

### 2. Compile the Server (without linking to the library)

The server doesn’t link to libkvstore.so at compile time — it loads it dynamically at runtime.
```bash

gcc server.c -o server -ldl
```

Explanation:

- ldl → Links with libdl, required for dlopen() / dlsym() / dlclose().

The server binary is independent of the .so file — it will look for it at runtime.

---

##  Understanding the Dynamic Linking

| Function | Purpose |
|:----------|:---------|
| `dlopen("libkvstore.so", RTLD_LAZY)` | Loads the shared library at runtime. |
| `dlsym(handle, "kv_get")` | Fetches a pointer to the function `kv_get()` from the library. |
| `dlerror()` | Returns any error message from the dynamic loader. |
| `dlclose(handle)` | Unloads the library when done. |


---

