#pragma once

#include <cstring>

#define GDT_PRESENT 0x80
#define GDT_EXECUTABLE 8
#define GDT_GROWDOWN 4
#define GDT_CONFORMING 4
#define GDT_RW 2
#define GDT_ACCESSED 1

#define GDT_RING(ring) ((ring) << 5) 

#define GDT_PAGE_GRANULARITY 8
#define GDT_32BIT 4
#define GDT_16BIT 0

typedef struct __attribute__ ((packed)) {
    uint16_t limit_lo;
    uint32_t base_lo : 24;
    
    union {
        struct {
            uint8_t accessed : 1;
            uint8_t rw : 1;
            uint8_t conforming : 1;
            uint8_t executable : 1;
            uint8_t always_1 : 1;
            uint8_t privl : 2;
            uint8_t present : 1;
        } access;
        uint8_t access_byte;
    };
    
    uint8_t limit_hi : 4;
    
    uint8_t zeroes : 2;
    uint8_t size : 1;
    uint8_t granularity : 1;
    
    uint8_t base_hi;
    
    void set_limit(uint32_t lim) { limit_lo = lim; limit_hi = lim >> 16; }
    void set_base(uint32_t base) { base_lo = base; base_hi = base >> 24; } 
    void set_no1(uint32_t base, uint32_t lim, uint8_t access_b, uint8_t flags) {
        set_base(base);
        set_limit(lim);
        access_byte = access_b;
        zeroes = 0;
        size = flags & GDT_32BIT ? 1 : 0;
        granularity = flags & GDT_PAGE_GRANULARITY ? 1 : 0;
    }
    void set(uint32_t base, uint32_t lim, uint8_t access_b, uint8_t flags) {
        set_no1(base, lim, access_b, flags);
        access.always_1 = 1;
    }
    void erase() {
        set_base(0);
        set_limit(0);
        access_byte = 0;
        zeroes = 0;
        size = 0;
        granularity = 0;
    }
    
} gdt_entry;

typedef struct __attribute__ ((packed)) {
    uint16_t limit;
    gdt_entry *base;
} gdtr;

typedef struct __attribute__ ((packed)) {
    uint32_t prev_tss;   // The previous TSS - if we used hardware task switching this would form a linked list.
    uint32_t esp0;       // The stack pointer to load when we change to kernel mode.
    uint32_t ss0;        // The stack segment to load when we change to kernel mode.
    uint32_t esp1;       // everything below here is unusued now.. 
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;         
    uint32_t cs;        
    uint32_t ss;        
    uint32_t ds;        
    uint32_t fs;       
    uint32_t gs;         
    uint32_t ldt;      
    uint16_t trap;
    uint16_t iomap_base;
} tss_entry;

#define GDT_ENTRY_COUNT 16
#define TSS_ENTRY_COUNT 1

typedef struct {
    gdtr reg;
    gdt_entry entries[GDT_ENTRY_COUNT];
    tss_entry tss_entries[TSS_ENTRY_COUNT];
} gdt_info;

class gdt {
public:
    static void init(gdt_info *info, void *tss_stack) {
        info->reg.base = info->entries;
        info->reg.limit = GDT_ENTRY_COUNT * sizeof(gdt_entry) - 1;
        
        int i = 0;
        
        info->entries[i++].erase();
        info->entries[i++].set(0, 0xffffffff, GDT_PRESENT | GDT_EXECUTABLE | GDT_RW | GDT_RING(0), GDT_32BIT | GDT_PAGE_GRANULARITY);
        info->entries[i++].set(0, 0xffffffff, GDT_PRESENT | GDT_RW | GDT_RING(0), GDT_32BIT | GDT_PAGE_GRANULARITY);
        info->entries[i++].set(0, 0xffffffff, GDT_PRESENT | GDT_EXECUTABLE | GDT_RW | GDT_RING(3), GDT_32BIT | GDT_PAGE_GRANULARITY);
        info->entries[i++].set(0, 0xffffffff, GDT_PRESENT | GDT_RW | GDT_RING(3), GDT_32BIT | GDT_PAGE_GRANULARITY);
        //info->entries[i++].set(0x40000000, 0xbfffffff, GDT_PRESENT | GDT_EXECUTABLE | GDT_RW | GDT_RING(3), GDT_32BIT | GDT_PAGE_GRANULARITY);
        //info->entries[i++].set(0x40000000, 0xbfffffff, GDT_PRESENT | GDT_RW | GDT_RING(3), GDT_32BIT | GDT_PAGE_GRANULARITY);
        
        // TSS
        info->entries[i++].set_no1(reinterpret_cast<uint32_t>(info->tss_entries), sizeof(tss_entry) * TSS_ENTRY_COUNT - 1, 
                                   GDT_PRESENT | GDT_ACCESSED | GDT_EXECUTABLE | GDT_RING(3), 0);
        
        for (; i < GDT_ENTRY_COUNT; i++) info->entries[i].erase();
        
        memset(info->tss_entries, 0, sizeof(tss_entry) * TSS_ENTRY_COUNT);
        info->tss_entries[0].ss0 = 0x10;
        info->tss_entries[0].esp0 = reinterpret_cast<uint32_t>(tss_stack);
        
        asm volatile ( "lgdt %[gdtr]\n"
                       "mov $0x28, %%ax\n"
                       "ltr %%ax\n" :: [gdtr] "m" (info->reg));
    }
};
