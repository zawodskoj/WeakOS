#pragma once

#include <cstdint>

#define ATEXIT_MAX_FUNCS 128

typedef uint32_t uarch_t;

struct atexit_func_entry_t
{
	/*
	* Each member is at least 4 bytes large. Such that each entry is 12bytes.
	* 128 * 12 = 1.5KB exact.
	**/
	void (*destructor_func)(void *);
	void *obj_ptr;
	void *dso_handle;
};

extern "C" void __cxa_pure_virtual();
extern "C" int __cxa_atexit(void (*destructor) (void *), void *arg, void *__dso_handle);
extern "C" void __cxa_finalize(void *f);
extern "C" void _start();
extern "C" void _init();
extern "C" void _fini();

extern "C" int main();
