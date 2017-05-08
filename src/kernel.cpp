#include <stdint.h>

#include <cpp.h>
#include <init.h>
#include <ustdio.h>
#include <os/memory.h>

#include <cstdio>

#include "malloc_test.inc"

extern "C" {
    int k_main();
}

const char *types[] = { "Usable", "Reserved", "ACPI reclaimable", "ACPI NVS", "Bad" };

typedef struct __attribute__((packed)) {
    uint64_t base;
    uint64_t page_count;
    uint32_t type;
} mem_rgn;

int k_main() {
    init();
    
    // stdio::printf("Zw\n");
    
    //for (int i = 0; i < 50; i++)
    //
    // stdio::printf("Test: %x\n", memory::alloc_virtual(237));
    
    malloc_test(true);
    
    // *(char*)0xb8000 = '!';
    while (1);
}
