#include <os/memory.h>
#include <os/paging.h>
#include <ustdio.h>


uint8_t *memory::m_pages;
mem_virt_rgn *memory::m_first_rgn;
mem_virt_rgn *memory::m_rgns;
paging_info *memory::m_paging;

uint32_t align_down(uint32_t val, uint32_t alignment) {
    return val - (val % alignment);
}

uint32_t align_up(uint32_t val, uint32_t alignment) {
    uint32_t remainder = val % alignment;
    return remainder ? val - remainder + alignment : val;
}

void memory::load_bios_rgns(mem_bios_rgn *rgns) {
    int cnt = *reinterpret_cast<int*>(0xa000);
    
    rgns->length = 0;
    
    for (int i = 0; i < cnt; i++) {
        mem_bios_rgn *rgn = reinterpret_cast<mem_bios_rgn*>(0xa020 + i * 0x20);
        rgns->base = rgn->base;
        rgns->length = rgn->length;
        rgns->type = rgn->type;
        rgns++;
    }
    for (int i = 0; i < mem_max_bios_rgns - cnt; i++) {
        rgns->base = rgns->length = 0;
        rgns->type = mem_bios_rgn_type::bad;
        rgns++;
    }
}

void memory::preinit(mem_info *info) {
    memory::m_pages = info->phys_pages;
    memory::m_rgns = info->virt_rgns;
    memory::load_bios_rgns(info->bios_rgns);
    
    memset(memory::m_pages, 0, sizeof(uint8_t) * mem_page_count / 8);
    memset(memory::m_rgns, 0, sizeof(mem_virt_rgn) * mem_page_count);
    
    for (int i = 0; i < mem_max_bios_rgns; i++) {
        mem_bios_rgn rgn = info->bios_rgns[i];
        
        if (rgn.type != mem_bios_rgn_type::usable) continue;
        if (rgn.length == 0) continue;
        if (rgn.base > 0xffffffffull) continue;
        if (rgn.base + rgn.length > 0xffffffffull) {
            rgn.length = 0xffffffffull - rgn.base + 1;
            // возможность переполнения uint32_t при base == 0
            // вероятность полного адресного пространства КРАЙНЕ МАЛА, поэтому можно забить
        }
        
        memory::enable_physical(rgn.base, rgn.length);
    }
    
    memory::m_first_rgn = memory::m_rgns;
    memory::m_rgns[0].type = mem_virt_rgn_type::free;
    memory::m_rgns[0].length = mem_page_count;
}

void memory::init(paging_info *paging) {
    memory::m_paging = paging;
}

void memory::enable_physical(uint32_t base, uint32_t length) {
    uint32_t max_page = align_up(base + length, 0x1000) / 0x1000;
    
    for (uint32_t page = base / 0x1000; page < max_page; page++) {
        memory::m_pages[page / 8] |= 1 << (page % 8);
    }
}

void memory::disable_physical(uint32_t base, uint32_t length) {     
    uint32_t max_page = align_up(base + length, 0x1000) / 0x1000;
    
    for (uint32_t page = base / 0x1000; page < max_page; page++) {
        memory::m_pages[page / 8] &= ~(1 << (page % 8));
    }
}

void mem_invlpg(uint32_t addr) {
    __asm__ volatile ( "invlpg (%0)" : : "b"(addr) : "memory" );
}

#define PAGEOF(page) (reinterpret_cast<page_table_entry*>(reinterpret_cast<uint32_t*>(memory::m_paging->tables) + page))

void memory::unmap_page(uint32_t page) {
    page_table_entry *ent = PAGEOF(page);
    
    uint32_t page_num = ent->address;
    memory::m_pages[page_num / 8] |= (1 << (page_num % 8));
    ent->flags_byte = 0;
    mem_invlpg(page_num * 0x1000);
}

bool memory::map_page(uint32_t page) {
    page_table_entry *ent = PAGEOF(page);
    
    for (uint32_t i = 0; i < mem_page_count; i++) {
        if (memory::m_pages[i / 8] & (1 << (i % 8))) {
            memory::m_pages[i / 8] &= ~(1 << (i % 8));
            ent->address = i;
            ent->flags_enum = pte_flags::generic_user;
            
            mem_invlpg(page * 0x1000);
            return true;
        }
    }
    stdio::printf("cannot map page\n");
    return false;
}

mem_virt_rgn* get_unused(mem_virt_rgn *rgns) {
    while (rgns->type != mem_virt_rgn_type::none) rgns++;
    return rgns;
}

void* memory::alloc_virtual(uint32_t pages) {
    auto rgn = memory::m_first_rgn;
    while (rgn) {
        // stdio::printf("av memrgn base %x len %x type %u\n", rgn->base, rgn->length, (uint32_t) rgn->type);
        if (rgn->type == mem_virt_rgn_type::free && rgn->length >= pages) {
            if (rgn->length > pages) {
                auto empty = get_unused(memory::m_rgns);
                empty->type = mem_virt_rgn_type::free;
                empty->base = rgn->base + pages;
                empty->length = rgn->length - pages;
                empty->next = rgn->next;
                empty->prev = rgn;
                
                rgn->length = pages;
                rgn->next = empty;
            }
            rgn->type = mem_virt_rgn_type::used;
            
            for (unsigned i = 0; i < rgn->length; i++) {
                if (!memory::map_page(rgn->base + i)) {
                    for (unsigned j = 0; j < i; j++)
                        memory::unmap_page(rgn->base + i);
                    return 0;
                }
            }
            
            return reinterpret_cast<void*>(rgn->base * 0x1000);
        }
        rgn = rgn->next;
    }
    return 0;
}

bool memory::free_virtual(void *ptr) {
    uint32_t addr = reinterpret_cast<uint32_t>(ptr);
    if (addr % 0x1000) return false; // invalid addr
    addr /= 0x1000;
    
    auto rgn = memory::m_first_rgn;
    while (rgn) {
        if (rgn->base == addr && rgn->type == mem_virt_rgn_type::used) { 
            for (unsigned i = 0; i < rgn->length; i++) memory::unmap_page(rgn->base + i);
            rgn->type = mem_virt_rgn_type::free;
            return true;
        }
        rgn = rgn->next;
    }
    return false;
}

void memory::disable_virtual(uint32_t addr, uint32_t length) {
    uint32_t pages = align_up(length, 0x1000) / 0x1000;
    if (addr % 0x1000) return; // invalid addr
    addr /= 0x1000;
    uint32_t endaddr = addr + pages;
    
    auto rgn = memory::m_first_rgn;
    while (rgn) {
        // stdio::printf("dv memrgn base %x len %x type %u\n", rgn->base, rgn->length, (uint32_t) rgn->type);
        if (rgn->type == mem_virt_rgn_type::disabled || rgn->type == mem_virt_rgn_type::none) goto next;
        if (rgn->base >= addr) { 
            if (rgn->base >= endaddr) goto next;
            if (rgn->base + rgn->length < endaddr) {
                rgn->type = mem_virt_rgn_type::disabled;
                goto next;
            }
            
            uint32_t oldlen = rgn->length;
            rgn->length = endaddr - 1 - rgn->base;
            
            auto empty = get_unused(memory::m_rgns);
            empty->type = mem_virt_rgn_type::free;
            empty->base = endaddr;
            empty->length = oldlen - rgn->length - 1;
            empty->next = rgn->next;
            empty->prev = rgn;
            empty->next->prev = empty;
            
            rgn->next = empty;
            rgn->type = mem_virt_rgn_type::disabled;
        } else {
            if (rgn->base + rgn->length - 1 < addr) goto next;
            if (rgn->base + rgn->length >= endaddr) {
                auto before = get_unused(memory::m_rgns);
                before->type = mem_virt_rgn_type::free;
                before->base = rgn->base;
                before->length = addr - rgn->base - 1;
                before->next = rgn;
                before->prev = rgn->prev;
                before->prev->next = before;
                
                if (rgn->base + rgn->length > endaddr) {
                    auto after = get_unused(memory::m_rgns);
                    after->type = mem_virt_rgn_type::free;
                    after->base = endaddr;
                    after->length = rgn->length - pages - before->length;
                    after->next = rgn->next;
                    after->next->prev = after;
                    after->prev = rgn;
                    
                    rgn->next = after;
                }
                
                rgn->base += before->length + 1;
                rgn->length = pages;
                rgn->prev = before;
                rgn->type = mem_virt_rgn_type::disabled;
                
                return;
            } else {
                auto before = get_unused(memory::m_rgns);
                before->type = mem_virt_rgn_type::free;
                before->base = rgn->base;
                before->length = addr - rgn->base;
                before->next = rgn;
                before->prev = rgn->prev;
                before->prev->next = before;
            
                rgn->prev = before;
                rgn->length -= before->length;
                rgn->type = mem_virt_rgn_type::disabled;
            }
        }
        next: rgn = rgn->next;
    }
}
