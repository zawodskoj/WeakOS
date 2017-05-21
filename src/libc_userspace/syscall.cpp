#include <os/syscall.h>

uint32_t syscall(uint32_t syscall_num, int argc, uint32_t *argv) {
    uint32_t out;
    
    asm volatile (
        "mov %1, %%ecx      \n\
        mov %2, %%eax       \n\
        test %%ecx, %%ecx   \n\
        jz 2f               \n\
        1: push (%%eax)     \n\
        add $4, %%eax       \n\
        loop 1b             \n\
        2: push %1          \n\
        push %3             \n\
        int $0xc9           \n\
        pop %%eax           \n\
        pop %%ecx           \n\
        test %%ecx, %%ecx   \n\
        jz 2f               \n\
        1: pop %%ebx        \n\
        loop 1b             \n\
        2: mov %%eax, %0" : "=r"(out) : "m" (argc), "m" (argv), "m" (syscall_num) : "eax", "ebx", "ecx", "cc" );
    
    return out;
    // asm volatile ( "push %0\npush $1\nint $0xc9\npop %%eax\npop %%eax" :: "m"(ch) );
}
