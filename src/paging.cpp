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

void paging::init(paging_info *info) {
    for (int i = 0; i < 1024; i++) {
        info->directory[i].flags_byte = PDE_PRESENT | PDE_WRITEABLE | PDE_USER_VISIBLE;
        info->directory[i].i_disabled = 0;
        info->directory[i].address = reinterpret_cast<uint32_t>(info->tables[i]) >> 12;
    }
    
    info->directory[0].flags_byte = PDE_PRESENT | PDE_WRITEABLE | PDE_USER_VISIBLE | PDE_PAGE_4MB;
    info->directory[0].address = 0;
    info->directory[0].i_disabled = 1;
    
    info->directory[1].flags_byte = PDE_PRESENT | PDE_WRITEABLE | PDE_USER_VISIBLE | PDE_PAGE_4MB;
    info->directory[1].address = PSE_ADDR(0x2000000);
    info->directory[1].i_disabled = 1;
    
    info->directory[2].flags_byte = PDE_PRESENT | PDE_WRITEABLE | PDE_USER_VISIBLE | PDE_PAGE_4MB;
    info->directory[2].address = PSE_ADDR(0x800000);
    info->directory[2].i_disabled = 1;
    
    info->directory[3].flags_byte = PDE_PRESENT | PDE_WRITEABLE | PDE_USER_VISIBLE | PDE_PAGE_4MB;
    info->directory[3].address = PSE_ADDR(0x1000000);
    info->directory[3].i_disabled = 1;
    
    info->directory[4].flags_byte = PDE_PRESENT | PDE_WRITEABLE | PDE_USER_VISIBLE | PDE_PAGE_4MB;
    info->directory[4].address = PSE_ADDR(0x1400000);
    info->directory[4].i_disabled = 1;
    
    info->directory[5].flags_byte = PDE_PRESENT | PDE_WRITEABLE | PDE_USER_VISIBLE | PDE_PAGE_4MB;
    info->directory[5].address = PSE_ADDR(0x1800000);
    info->directory[5].i_disabled = 1;
    
    info->directory[6].flags_byte = PDE_PRESENT | PDE_WRITEABLE | PDE_USER_VISIBLE | PDE_PAGE_4MB;
    info->directory[6].address = PSE_ADDR(0x1c00000);
    info->directory[6].i_disabled = 1;
    
    info->directory[7].flags_byte = PDE_PRESENT | PDE_WRITEABLE | PDE_USER_VISIBLE | PDE_PAGE_4MB;
    info->directory[7].address = PSE_ADDR(0x2400000);
    info->directory[7].i_disabled = 1;
    
    info->directory[8].flags_byte = PDE_PRESENT | PDE_WRITEABLE | PDE_USER_VISIBLE | PDE_PAGE_4MB;
    info->directory[8].address = PSE_ADDR(0x2800000);
    info->directory[8].i_disabled = 1;
    
    info->directory[9].flags_byte = PDE_PRESENT | PDE_WRITEABLE | PDE_USER_VISIBLE | PDE_PAGE_4MB;
    info->directory[9].address = PSE_ADDR(0x2c00000);
    info->directory[9].i_disabled = 1;
    
    info->directory[10].flags_byte = PDE_PRESENT | PDE_WRITEABLE | PDE_USER_VISIBLE | PDE_PAGE_4MB;
    info->directory[10].address = PSE_ADDR(0xc00000);
    info->directory[10].i_disabled = 1;
    
    info->directory[DIRECTORY_FOR(0xc0000000)].flags_byte = PDE_PRESENT | PDE_WRITEABLE | PDE_USER_VISIBLE | PDE_PAGE_4MB;
    info->directory[DIRECTORY_FOR(0xc0000000)].address = PSE_ADDR(0x400000);
    info->directory[DIRECTORY_FOR(0xc0000000)].i_disabled = 1;
    
    paging::enable(&info->directory);
}
