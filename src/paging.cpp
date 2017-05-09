#include <os/paging.h>

void paging::enable(void *directory) {
    __asm__ volatile( 
        "mov %%cr4, %%eax\n"
        "bts $4, %%eax\n"
        "mov %%eax, %%cr4\n"
        "mov %0, %%eax\n"
        "mov %%eax, %%cr3\n"
        "mov %%cr0, %%eax\n"
        "or $0x80000001, %%eax\n"
        "mov %%eax, %%cr0\n" :: "m"(directory) );
}

void set_directory(page_directory_entry &pde, uint8_t flags, uint32_t addr) {
    pde.flags_byte = flags;
    pde.address = addr;
}

#define PDE_GENERIC PDE_PRESENT | PDE_WRITEABLE | PDE_USER_VISIBLE | PDE_PAGE_4MB

void paging::init(paging_info *info) {
    for (int i = 0; i < 1024; i++) {
        info->directory[i].flags_byte = PDE_PRESENT | PDE_WRITEABLE | PDE_USER_VISIBLE;
        info->directory[i].i_disabled = 0;
        info->directory[i].address = reinterpret_cast<uint32_t>(info->tables[i]) >> 12;
    }
    
    set_directory(info->directory[0], PDE_GENERIC,  0);
    set_directory(info->directory[1], PDE_GENERIC,  PSE_ADDR(0x0800000));
    set_directory(info->directory[2], PDE_GENERIC,  PSE_ADDR(0x0c00000));
    set_directory(info->directory[3], PDE_GENERIC,  PSE_ADDR(0x1000000));
    set_directory(info->directory[4], PDE_GENERIC,  PSE_ADDR(0x1400000));
    set_directory(info->directory[5], PDE_GENERIC,  PSE_ADDR(0x1800000));
    set_directory(info->directory[6], PDE_GENERIC,  PSE_ADDR(0x1c00000));
    set_directory(info->directory[7], PDE_GENERIC,  PSE_ADDR(0x2000000));
    set_directory(info->directory[8], PDE_GENERIC,  PSE_ADDR(0x2400000));
    set_directory(info->directory[9], PDE_GENERIC,  PSE_ADDR(0x2800000));
    set_directory(info->directory[10], PDE_GENERIC, PSE_ADDR(0x2c00000));
    set_directory(info->directory[11], PDE_GENERIC, PSE_ADDR(0x3000000));
    set_directory(info->directory[12], PDE_GENERIC, PSE_ADDR(0x3400000));
    set_directory(info->directory[13], PDE_GENERIC, PSE_ADDR(0x3800000));
    set_directory(info->directory[14], PDE_GENERIC, PSE_ADDR(0x3c00000));
    
    set_directory(info->directory[DIRECTORY_FOR(0xc0000000)], PDE_GENERIC, PSE_ADDR(0x400000));
    
    paging::enable(&info->directory);
}
