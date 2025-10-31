#ifndef KVSTORE_H
#define KVSTORE_H

#define BUF_SIZE 256

#ifdef __cplusplus
extern "C"
{
#endif

    const char *kv_get(const char *key);
    void kv_set(const char *key, const char *value);

#ifdef __cplusplus
}
#endif

#endif
