#include "elf_abi.h"
#include "curl_defs.h"
#include "common.h"
#include "utils.h"
#include "structs.h"
#include "elf_loading.h"

static int curl_write_data(void *buffer, int size, int nmemb, void *userp)
{
    curl_data_t *curl_data = userp;
    private_data_t *private_data = curl_data->private_data;

    int insize = size * nmemb;

    // check if buffer reallocation is needed
    if ((curl_data->downloadedSize + insize) > curl_data->curlBufferSize)
    {
        // allocate 4k chunks to limit the number of reallocations
        void *oldPtr = curl_data->curlBufferPtr;
        curl_data->curlBufferSize += 0x1000;
        curl_data->curlBufferPtr = private_data->MEMAllocFromDefaultHeapEx(curl_data->curlBufferSize, 0x40);
        private_data->memcpy(curl_data->curlBufferPtr, oldPtr, curl_data->downloadedSize);
        private_data->MEMFreeToDefaultHeap(oldPtr);
    }

    // append the downloaded data to the file buffer
    private_data->memcpy(curl_data->curlBufferPtr + curl_data->downloadedSize, buffer, insize);
    curl_data->downloadedSize += insize;

    return insize;
}

static int32_t DownloadFileToMem(private_data_t *private_data, const char *fileurl, uint8_t **fileOut, uint32_t * sizeOut) {
    curl_data_t curl_data;

    // allocate an initial 4k buffer
    curl_data.private_data = private_data;
    curl_data.curlBufferSize = 0x1000;
    curl_data.curlBufferPtr = private_data->MEMAllocFromDefaultHeapEx(curl_data.curlBufferSize, 0x40);
    curl_data.downloadedSize = 0;

    if (!curl_data.curlBufferPtr) {
        return 0;
    }

    // initialize curl library
    void *curl = private_data->curl_easy_init();
    if(!curl) {
        OSFatal("curl_easy_init failed.");
    }

    // prepare for download
    private_data->curl_easy_setopt(curl, CURLOPT_URL, (char*)fileurl);
    private_data->curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_data);
    private_data->curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curl_data);

    // download the file
    int ret = private_data->curl_easy_perform(curl);
    if(ret) {
        OSFatal("curl_easy_perform failed.");
    }

    // check response code
    int resp = 404;
    private_data->curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp);
    if(resp != 200) {
        char buf[0x255];
        __os_snprintf(buf,0x254,"File download failed. (error %d)", resp);
        OSFatal(buf);
    }

    // cleanup
    private_data->curl_easy_cleanup(curl);

    // done
    *fileOut = (uint8_t*)curl_data.curlBufferPtr;
    *sizeOut = curl_data.downloadedSize;

    return 1;
}

static uint32_t load_elf_image_to_mem (private_data_t *private_data, uint8_t *elfstart) {
    Elf32_Ehdr *ehdr;
    Elf32_Phdr *phdrs;
    uint8_t *image;
    int32_t i;

    ehdr = (Elf32_Ehdr *) elfstart;

    if(ehdr->e_phoff == 0 || ehdr->e_phnum == 0) {
        return 0;
    }

    if(ehdr->e_phentsize != sizeof(Elf32_Phdr)) {
        return 0;
    }

    phdrs = (Elf32_Phdr*)(elfstart + ehdr->e_phoff);

    for(i = 0; i < ehdr->e_phnum; i++) {
        if(phdrs[i].p_type != PT_LOAD) {
            continue;
        }

        if(phdrs[i].p_filesz > phdrs[i].p_memsz) {
            continue;
        }

        if(!phdrs[i].p_filesz) {
            continue;
        }

        uint32_t p_paddr = phdrs[i].p_paddr;
        image = (uint8_t *) (elfstart + phdrs[i].p_offset);

        private_data->memcpy ((void *) p_paddr, image, phdrs[i].p_filesz);
        private_data->DCFlushRange((void*)p_paddr, phdrs[i].p_filesz);

        if(phdrs[i].p_flags & PF_X) {
            private_data->ICInvalidateRange ((void *) p_paddr, phdrs[i].p_memsz);
        }
    }

    //! clear BSS
    Elf32_Shdr *shdr = (Elf32_Shdr *) (elfstart + ehdr->e_shoff);
    for(i = 0; i < ehdr->e_shnum; i++) {
        const char *section_name = ((const char*)elfstart) + shdr[ehdr->e_shstrndx].sh_offset + shdr[i].sh_name;
        if(section_name[0] == '.' && section_name[1] == 'b' && section_name[2] == 's' && section_name[3] == 's') {
            private_data->memset((void*)shdr[i].sh_addr, 0, shdr[i].sh_size);
            private_data->DCFlushRange((void*)shdr[i].sh_addr, shdr[i].sh_size);
        } else if(section_name[0] == '.' && section_name[1] == 's' && section_name[2] == 'b' && section_name[3] == 's' && section_name[4] == 's') {
            private_data->memset((void*)shdr[i].sh_addr, 0, shdr[i].sh_size);
            private_data->DCFlushRange((void*)shdr[i].sh_addr, shdr[i].sh_size);
        }
    }

    return ehdr->e_entry;
}

uint32_t DownloadAndCopyFile(const char *fileurl) {
    private_data_t private_data;

    loadFunctionPointers(&private_data);

    unsigned char *pElfBuffer = NULL;
    unsigned int uiElfSize = 0;

    DownloadFileToMem(&private_data, fileurl, &pElfBuffer, &uiElfSize);

    if(!pElfBuffer) {
        OSFatal("Failed to download payload.elf");
    }
    unsigned int newEntry = load_elf_image_to_mem(&private_data, pElfBuffer);
    if(newEntry == 0) {
        OSFatal("Failed to load elf");
    }

    private_data.MEMFreeToDefaultHeap(pElfBuffer);
    return newEntry;
}
