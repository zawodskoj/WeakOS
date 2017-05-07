#pragma once

#include <stddef.h>

extern "C" void* malloc(size_t size);
extern "C" void* calloc(size_t num, size_t size);
extern "C" void* realloc(void *ptr, size_t size);
extern "C" void  free(void *ptr);
