#include <ustdio.h>

#include <os/fat.h>
#include <os/elf.h>
#include <os/process.h>
#include <os/syscall.h>

#include <string>

fat *syscall::xfat;

int main() {
    syscall::xfat = new fat();
    uint32_t size = syscall::xfat->get_file_size("test.elf");
    stdio::printf("elf file size: %u\n", size);
    uint8_t *elf_data = new uint8_t[size];    
    syscall::xfat->get_file("testdir/test.elf", elf_data);
    stdio::printf("loaded!\n");
    
    elf *v_elf = new elf(elf_data);
    process *p = new process(v_elf->m_entry);
    p->run();
    
    //v_elf->run();
    stdio::printf("done!\n");
}
