#pragma once

#define PAGING_PLS_NO_USE_THAT_MEM_SIZE 0x400000

#include <stdint.h>
#include <cpp.h>
#include <os/macro.h>

enum class pde_flags : uint8_t { 
    present = 1, 
    writeable = 2, 
    user_visible = 4, 
    write_through = 8,
    disable_cache = 16,
    page_4mb = 128,
    
    generic = present | writeable,
    generic_4mb = page_4mb | present | writeable,
    generic_user = present | writeable | user_visible,
    generic_user_4mb = page_4mb | present | writeable | user_visible
};

enum class pte_flags : uint8_t {
    present = 1,
    writeable = 2,
    user_visible = 4,
    write_through = 8,
    disable_cache = 16,
    
    generic = present | writeable,
    generic_user = present | writeable | user_visible
};

typedef struct __attribute__((packed)) {
    uint8_t present : 1;
    uint8_t writeable : 1;
    uint8_t user_visible : 1;
    uint8_t write_through : 1;
    uint8_t cache_disabled : 1;
    uint8_t accessed : 1;
    uint8_t zero : 1;
    uint8_t page_size : 1;
} page_directory_entry_flags;

typedef struct __attribute__((packed)) {
    union {
        page_directory_entry_flags flags;
        uint8_t flags_byte;
        pde_flags flags_enum;
    };
    
    uint8_t ignored : 4;
    
    uint32_t address : 20;
} page_directory_entry;

typedef struct __attribute__((packed)) {
    uint8_t present : 1;
    uint8_t writeable : 1;
    uint8_t user_visible : 1;
    uint8_t write_through : 1;
    uint8_t cache_disabled : 1;
    uint8_t accessed : 1;
    uint8_t dirty : 1;
    uint8_t zero : 1;
} page_table_entry_flags;

typedef struct __attribute__((packed)) {
    union {
        page_table_entry_flags flags;
        uint8_t flags_byte;
        pte_flags flags_enum;
    };
    
    uint8_t global : 1;
    uint8_t ignored : 3;
    
    uint32_t address : 20;
} page_table_entry;

const int page_directory_entry_count = 0x400;
const int page_table_entry_count = 0x400;
const int page_structure_alignment = 0x1000;

#define DIRECTORY_FOR(addr) (static_cast<uint32_t>(addr) >> 22)
#define PSE_ADDR(addr) ((static_cast<uint32_t>(addr) & 0xffc00000) >> 12)

typedef page_directory_entry page_directory[page_directory_entry_count];
typedef page_table_entry page_table[page_table_entry_count] __attribute__((aligned(page_structure_alignment)));

typedef struct {
    page_directory directory __attribute__((aligned(page_structure_alignment)));
    page_table tables[page_directory_entry_count];
} paging_info;

class paging {
private:
    static void enable(uint32_t directory);
public:
    static void init(paging_info *info);
};
