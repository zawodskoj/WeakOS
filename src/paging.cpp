#include <os/paging.h>

void paging::enable(uint32_t directory) {
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

void set_directory(page_directory_entry &pde, pde_flags flags, uint32_t addr) {
    pde.flags_enum = flags;
    pde.address = addr;
}

void paging::init(paging_info *info) {
    for (int i = 0; i < 1024; i++) {
        info->directory[i].flags_enum = i >= 0x100 && i < 0x300 ? pde_flags::generic_user : pde_flags::generic;
        info->directory[i].address = (reinterpret_cast<uint32_t>(info->tables[i]) + 0x800000) >> 12;
    }
    
    set_directory(info->directory[0], pde_flags::generic_4mb,  0);
    set_directory(info->directory[1], pde_flags::generic_4mb,  PSE_ADDR(0x0c00000));
    set_directory(info->directory[2], pde_flags::generic_4mb,  PSE_ADDR(0x1000000));
    set_directory(info->directory[3], pde_flags::generic_4mb,  PSE_ADDR(0x1400000));
    set_directory(info->directory[4], pde_flags::generic_4mb,  PSE_ADDR(0x1800000));
    set_directory(info->directory[5], pde_flags::generic_4mb,  PSE_ADDR(0x1c00000));
    set_directory(info->directory[6], pde_flags::generic_4mb,  PSE_ADDR(0x2000000));
    set_directory(info->directory[7], pde_flags::generic_4mb,  PSE_ADDR(0x2400000));
    set_directory(info->directory[8], pde_flags::generic_4mb,  PSE_ADDR(0x2800000));
    set_directory(info->directory[9], pde_flags::generic_4mb,  PSE_ADDR(0x2c00000));
    set_directory(info->directory[10], pde_flags::generic_4mb, PSE_ADDR(0x3000000));
    set_directory(info->directory[11], pde_flags::generic_4mb, PSE_ADDR(0x3400000));
    set_directory(info->directory[12], pde_flags::generic_4mb, PSE_ADDR(0x3800000));
    set_directory(info->directory[13], pde_flags::generic_4mb, PSE_ADDR(0x3c00000));
    set_directory(info->directory[14], pde_flags::generic_4mb, PSE_ADDR(0x4000000));
    
    set_directory(info->directory[DIRECTORY_FOR(0xc0000000)], pde_flags::generic_4mb, PSE_ADDR(0x400000));
    set_directory(info->directory[DIRECTORY_FOR(0xffc00000)], pde_flags::generic_4mb, PSE_ADDR(0x800000));
    
    set_directory(info->directory[DIRECTORY_FOR(0x80000000)], pde_flags::generic_user_4mb, PSE_ADDR(0x4400000));
    set_directory(info->directory[DIRECTORY_FOR(0x5fc00000)], pde_flags::generic_user_4mb, PSE_ADDR(0x4800000));
    
    uint32_t directory_virtual = reinterpret_cast<uint32_t>(&info->directory);
    uint32_t directory_physical = directory_virtual + 0x800000;
    
    paging::enable(directory_physical);
}
