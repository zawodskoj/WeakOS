#include <stddef.h>
#include <os/syscall.h>

extern "C" void* malloc (size_t size) {
    uint32_t syscall_arg[1];
    syscall_arg[0] = (uint32_t) size;
    return (void*) syscall(3, 1, syscall_arg);
}
