#ifndef STRUCTS_H
#define    STRUCTS_H

#include <stdint.h>
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _private_data_t {
    EXPORT_DECL(void *, MEMAllocFromDefaultHeapEx,int size, int align);
    EXPORT_DECL(void, MEMFreeToDefaultHeap,void *ptr);

    EXPORT_DECL(void*, memcpy, void *p1, const void *p2, unsigned int s);
    EXPORT_DECL(void*, memset, void *p1, int val, unsigned int s);

    EXPORT_DECL(void, DCFlushRange, const void *addr, unsigned int length);
    EXPORT_DECL(void, ICInvalidateRange, const void *addr, unsigned int length);

    EXPORT_DECL(void*, curl_easy_init, void);
    EXPORT_DECL(void, curl_easy_setopt, void *handle, unsigned int param, void *op);
    EXPORT_DECL(void, curl_easy_getinfo, void *handle, unsigned int param, void *op);
    EXPORT_DECL(void, curl_easy_cleanup, void *handle);
    EXPORT_DECL(int, curl_easy_perform, void *handle);
} private_data_t;

typedef struct _curl_data_t {
    private_data_t *private_data;
	
    int curlBufferSize;
    void *curlBufferPtr;
	
    int downloadedSize;
} curl_data_t;

#ifdef __cplusplus
}
#endif

#endif    /* STRUCTS_H */

