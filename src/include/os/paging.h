#pragma once

#include <stdint.h>
#include <cpp.h>

#define PDE_PRESENT 1
#define PDE_WRITEABLE 2
#define PDE_USER_VISIBLE 4
#define PDE_WRITE_THROUGH 8
#define PDE_DISABLE_CACHE 16
#define PDE_PAGE_4MB 128

#define PTE_PRESENT 1
#define PTE_WRITEABLE 2
#define PTE_USER_VISIBLE 4
#define PTE_WRITE_THROUGH 8
#define PTE_DISABLE_CACHE 16

typedef struct __attribute__ ((packed)) {
    uint8_t present : 1;
    uint8_t writeable : 1;
    uint8_t user_visible : 1;
    uint8_t write_through : 1;
    uint8_t cache_disabled : 1;
    uint8_t accessed : 1;
    uint8_t zero : 1;
    uint8_t page_size : 1;
} page_directory_entry_flags;

typedef struct __attribute__ ((packed)) {
    union {
        page_directory_entry_flags flags;
        uint8_t flags_byte;
    };
    
    uint8_t ignored : 1;
    
    uint8_t i_free : 1;
    uint8_t i_disabled : 1;
    uint8_t i_rsvd : 1;
//    uint8_t ignored : 4;
    
    uint32_t address : 20;
} page_directory_entry;

typedef struct __attribute__ ((packed)) {
    uint8_t present : 1;
    uint8_t writeable : 1;
    uint8_t user_visible : 1;
    uint8_t write_through : 1;
    uint8_t cache_disabled : 1;
    uint8_t accessed : 1;
    uint8_t dirty : 1;
    uint8_t zero : 1;
} page_table_entry_flags;

typedef struct __attribute__ ((packed)) {
    union {
        page_table_entry_flags flags;
        uint8_t flags_byte;
    };
    
    uint8_t global : 1;
    
    
    uint8_t i_free : 1;
    uint8_t i_disabled : 1;
    uint8_t i_rsvd : 1;
    //uint8_t ignored : 3;
    
    uint32_t address : 20;
} page_table_entry;

#define PAGE_DIRECTORY_ENTRY_COUNT 0x400
#define PAGE_TABLE_ENTRY_COUNT 0x400

#define DIRECTORY_FOR(addr) (static_cast<uint32_t>(addr) >> 22)
#define PSE_ADDR(addr) ((static_cast<uint32_t>(addr) & 0xffc00000) >> 12)

typedef page_directory_entry page_directory[PAGE_DIRECTORY_ENTRY_COUNT];
typedef page_table_entry page_table[PAGE_TABLE_ENTRY_COUNT] __attribute__ ((__aligned__(0x1000)));

typedef struct {
    page_directory directory __attribute__ ((__aligned__(0x1000)));
    page_table tables[PAGE_DIRECTORY_ENTRY_COUNT];
} paging_info;

class paging {
private:
    static void enable(void *directory);
public:
    static void init(paging_info *info);
};
