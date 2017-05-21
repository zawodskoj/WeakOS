#pragma once

#include <cstdint>

typedef struct __attribute__ ((packed)) {
    uint32_t magic;
    uint8_t bits;
    uint8_t endianness;
    uint8_t elf_ver;
    uint8_t os_abi;
    uint8_t padding[8];
    uint16_t type;
    uint16_t instruction_set;
    uint32_t elf_ver_2;
    uint32_t entry_point;
    uint32_t header_pos;
    uint32_t sec_header_pos;
    uint32_t flags;
    uint16_t header_size;
    uint16_t header_entry_size;
    uint16_t header_entry_count;
    uint16_t sec_header_entry_size;
    uint16_t sec_header_entry_count;
    uint16_t section_names_index;
} elf_header;

typedef struct __attribute__ ((packed)) {
    uint32_t type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t reserved;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t flags;
    uint32_t alignment;
} elf_prog_32_header;

typedef int __cdecl (*entry_point)(int argc, char **argv);

class elf {
private:

    static inline void run_usermode(entry_point entry, uint32_t esp) {
        asm volatile ( 
            "mov $0x23, %%ax\n" 
            "mov %%ax, %%ds\n" 
            "mov %%ax, %%es\n" 
            "mov %1, %%eax\n"
            "push $0x23\n" 
            "push %%eax\n"
            "pushf\n"
            "push $0x1b\n"
            "push %0\n"
            "iret\n" :: "m" (entry), "m" (esp));
    }
    
public:
    entry_point m_entry;
    
    elf(uint8_t *elf_data) {
        auto header = reinterpret_cast<elf_header*>(elf_data);
        m_entry = reinterpret_cast<entry_point>(header->entry_point);
        
        /*stdio::printf("sig: %s\n", ((char*) header) + 1);
        stdio::printf("sig2: %x\n", *((uint32_t*) header));*/
        
        auto pheader = reinterpret_cast<elf_prog_32_header*>(elf_data + header->header_pos);
        
        for (int entry = 0; entry < header->header_entry_count; entry++) {
            if (pheader[entry].type == 1) {
                memset(reinterpret_cast<void*>(pheader[entry].p_vaddr), 0, pheader[entry].p_memsz);
                memcpy(reinterpret_cast<void*>(pheader[entry].p_vaddr), 
                    elf_data + pheader[entry].p_offset, pheader[entry].p_filesz);
            }
        }
        
        // stdio::printf("entry: %x", (void*) m_entry);
    }
    
    void run() {
        run_usermode(m_entry, 0x5ffffff0);
    }
};
