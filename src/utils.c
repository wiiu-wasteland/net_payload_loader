#include "utils.h"

void loadFunctionPointers(private_data_t * private_data) {
    unsigned int coreinit_handle;

    OSDynLoad_Acquire("coreinit", &coreinit_handle);

    unsigned int *functionPtr = 0;

    OSDynLoad_FindExport(coreinit_handle, 1, "MEMAllocFromDefaultHeapEx", &functionPtr);
    private_data->MEMAllocFromDefaultHeapEx = (void * (*)(int, int))*functionPtr;
    OSDynLoad_FindExport(coreinit_handle, 1, "MEMFreeToDefaultHeap", &functionPtr);
    private_data->MEMFreeToDefaultHeap = (void (*)(void *))*functionPtr;

    OS_FIND_EXPORT(coreinit_handle, "memcpy", private_data->memcpy);
    OS_FIND_EXPORT(coreinit_handle, "memset", private_data->memset);
    OS_FIND_EXPORT(coreinit_handle, "DCFlushRange", private_data->DCFlushRange);
    OS_FIND_EXPORT(coreinit_handle, "ICInvalidateRange", private_data->ICInvalidateRange);

    unsigned int nlibcurl_handle;
	OSDynLoad_Acquire("nlibcurl", &nlibcurl_handle);
	OS_FIND_EXPORT(nlibcurl_handle, "curl_easy_init", private_data->curl_easy_init);
	OS_FIND_EXPORT(nlibcurl_handle, "curl_easy_setopt", private_data->curl_easy_setopt);
	OS_FIND_EXPORT(nlibcurl_handle, "curl_easy_getinfo", private_data->curl_easy_getinfo);
	OS_FIND_EXPORT(nlibcurl_handle, "curl_easy_cleanup", private_data->curl_easy_cleanup);
	OS_FIND_EXPORT(nlibcurl_handle, "curl_easy_perform", private_data->curl_easy_perform);
}

/* Read a 32-bit word with kernel permissions */
uint32_t __attribute__ ((noinline)) kern_read(const void *addr) {
    uint32_t result;
    asm volatile (
        "li 3,1\n"
        "li 4,0\n"
        "li 5,0\n"
        "li 6,0\n"
        "li 7,0\n"
        "lis 8,1\n"
        "mr 9,%1\n"
        "li 0,0x3400\n"
        "mr %0,1\n"
        "sc\n"
        "nop\n"
        "mr 1,%0\n"
        "mr %0,3\n"
        :    "=r"(result)
        :    "b"(addr)
        :    "memory", "ctr", "lr", "0", "3", "4", "5", "6", "7", "8", "9", "10",
        "11", "12"
    );

    return result;
}

/* Write a 32-bit word with kernel permissions */
void __attribute__ ((noinline)) kern_write(void *addr, uint32_t value) {
    asm volatile (
        "li 3,1\n"
        "li 4,0\n"
        "mr 5,%1\n"
        "li 6,0\n"
        "li 7,0\n"
        "lis 8,1\n"
        "mr 9,%0\n"
        "mr %1,1\n"
        "li 0,0x3500\n"
        "sc\n"
        "nop\n"
        "mr 1,%1\n"
        :
        :    "r"(addr), "r"(value)
        :    "memory", "ctr", "lr", "0", "3", "4", "5", "6", "7", "8", "9", "10",
        "11", "12"
    );
}
