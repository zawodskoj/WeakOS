#include <cpp/new.h>

void * operator new(size_t size, void *ptr) {
    if (size == 0) size = 1;
    return ptr;
}
void * operator new(size_t size) {
    if (size == 0) size = 1;
    return malloc(size);
}

void operator delete(void *ptr) noexcept {}

