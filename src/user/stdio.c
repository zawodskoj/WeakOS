#include <stdio.h>
#include <syscall.h>

int printf(const char *format, ...) {
    syscall(SYS_print, format);
    return 0;
}
