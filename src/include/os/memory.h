#pragma once

#include <stddef.h>
#include <os/paging.h>

enum struct mem_bios_rgn_type : uint32_t { usable = 1, reserved = 2, acpi_reclaimable = 3, acpi_nvs = 4, bad = 5 };

typedef struct __attribute__((packed)) {
    uint64_t base;
    uint64_t length;
    mem_bios_rgn_type type;
} mem_bios_rgn;

enum struct mem_virt_rgn_type : uint32_t { none = 0, free = 1, used = 2, disabled = 3 };

typedef struct mem_virt_rgn {
    struct mem_virt_rgn *prev;
    mem_virt_rgn_type type;
    uint32_t base;
    uint32_t length;
    struct mem_virt_rgn *next;
} mem_virt_rgn;

const int mem_max_bios_rgns = 0x100;
const int mem_page_count = (1u << 31) / 0x1000 * 2; // 4GB address space / 4kb page
const int mem_max_virt_rgns = mem_page_count;

typedef struct {
    mem_bios_rgn bios_rgns[mem_max_bios_rgns];
    uint8_t phys_pages[mem_page_count / 8];
    mem_virt_rgn virt_rgns[mem_page_count];
} mem_info;

struct memory {
private:
    static uint8_t *m_pages;
    static paging_info *m_paging;
    static mem_virt_rgn *m_rgns;
    static mem_virt_rgn *m_first_rgn;
    static void load_bios_rgns(mem_bios_rgn *rgns);
    static bool map_page(uint32_t page);
    static void unmap_page(uint32_t page);
    
public:
    static void preinit(mem_info *info);
    static void init(paging_info *info);
    
    static void enable_physical(uint32_t base, uint32_t length);
    static void disable_physical(uint32_t base, uint32_t length);
    
    static void enable_virtual(uint32_t base, uint32_t length);
    static void disable_virtual(uint32_t base, uint32_t length);
    
    static void* alloc_virtual(uint32_t page_count);
    static bool free_virtual(void *ptr);
};

extern "C" void* memset(void *ptr, int c, size_t sz);
