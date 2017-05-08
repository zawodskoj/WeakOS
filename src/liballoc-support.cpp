#include <os/liballoc.h>
#include <os/memory.h>
#include <ustdio.h>

int liballoc_lock() { return 0; }
int liballoc_unlock() { return 0; }
void* liballoc_alloc(int pages) { 
    void *av =  memory::alloc_virtual(pages);
    if (av == 0) stdio::printf("alloc virtual for %d pages == 0\n", pages);
    return av;
    
}
int liballoc_free(void* ptr, int pages) { return !memory::free_virtual(ptr); }
