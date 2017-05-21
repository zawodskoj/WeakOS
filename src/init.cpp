#include <os/init.h>
#include <ustdio.h>

/*
     const int fixup_size = 0x200;
    
    uint32_t esp, ebp;
    
    asm volatile ( "mov %%esp, %0\n"
                   "mov %%ebp, %1\n": "=r"(esp), "=r"(ebp) );
    
    uint32_t *p_esp = reinterpret_cast<uint32_t*>(esp);
    
    for (int i = 0; i < fixup_size; i++) new_stack[KERNEL_STACK_SIZE - fixup_size + i] = p_esp[i];
    
    uint32_t new_esp = reinterpret_cast<uint32_t>(new_stack + KERNEL_STACK_SIZE - fixup_size);
    uint32_t new_ebp = ebp + new_esp - esp;
    
    asm volatile ( "mov %0, %%esp\n"
                   "mov %1, %%ebp\n":: "r"(new_esp), "r"(new_ebp) );
 */

void sysinit() {    
    kernel_data *data = reinterpret_cast<kernel_data*>(0);
    
    paging::init(&data->paging);
    
    stdio::init();
    
    gdt::init(&data->gdt, data->isr_stack + KERNEL_ISR_STACK_SIZE);
    
    memory::preinit(&data->mem);
    
    memory::init(&data->paging);
    
    memory::disable_physical(reinterpret_cast<uint32_t>(data), sizeof(kernel_data));
    memory::disable_virtual(0, 0x40000000);
    memory::disable_virtual(0x80000000, 0x40000000);
    memory::disable_virtual(0xc0000000, 0x40000000);
    
    interrupt::init(&data->idt);
    syscall::init();
    keyboard::init();
    
    ata::init();
    // time::init();
}
