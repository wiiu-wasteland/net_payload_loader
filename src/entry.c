#include "elf_abi.h"
#include "curl_defs.h"
#include "common.h"
#include "utils.h"
#include "structs.h"
#include "elf_loading.h"
#include "memory_setup.h"

int _start(int argc, char **argv) {
    setup_memory();

    uint32_t newEntry = DownloadAndCopyFile(PAYLOAD_URL);

    return ((int (*)(int, char **))newEntry)(argc, argv);
}
