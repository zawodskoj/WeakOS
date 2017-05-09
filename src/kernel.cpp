#include <ustdio.h>

#include <os/ata.h>
#include <os/time.h>
#include <os/interrupt.h>

volatile uint64_t time::m_ticks;

/*static inline void switch_usermode() {
    asm volatile ( 
        "mov $0x23, %ax\n" 
        "mov %ax, %ds\n" 
        "mov %ax, %es\n" 
        "mov %esp, %eax\n" 
        "push $0x23\n" 
        "push %eax\n"
        "pushf\n"
        "push $0x1b\n"
        "push $1f\n"
        "iret\n"
        "1: \n" );
}*/

int main() {
    stdio::printf("Hello!");
}
