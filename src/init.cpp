#include <init.h>
#include <stdio.h>

#include <os/paging.h>
#include <os/keyboard.h>
#include <os/interrupt.h>
#include <os/memory.h>

#define KMEM_RESERVED_BYTES 0x800000

typedef struct {
    uint8_t rsvd[KMEM_RESERVED_BYTES];
    paging_info paging;
    idt_info idt;
    mem_info mem;
} kernel_data;

void init() {
    kernel_data *data = reinterpret_cast<kernel_data*>(0);
    paging::init(&data->paging);
    
    stdio::init();
    
    memory::preinit(&data->mem);
    
    memory::init(&data->paging);
    
    memory::disable_physical(reinterpret_cast<uint32_t>(data), sizeof(kernel_data));
    memory::disable_virtual(0, 0x40000000);
    memory::disable_virtual(0xc0000000, 0x40000000);
    
    interrupt::init(&data->idt);
    keyboard::init();
}
