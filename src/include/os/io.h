#pragma once

#include <stdint.h>

#define INOUTF(suffix, size) static inline void out##suffix(uint16_t port, size val)\
{\
    __asm__ volatile ( "out" #suffix " %0, %1" : : "a"(val), "Nd"(port) );\
}\
static inline size in##suffix(uint16_t port)\
{\
    size ret;\
    __asm__ volatile ( "in" #suffix " %1, %0"\
                       : "=a"(ret)\
                       : "Nd"(port) );\
    return ret;\
}

class io {
public:
    INOUTF(b, uint8_t)
    INOUTF(w, uint16_t)
    INOUTF(l, uint32_t)
    
    static inline void wait() {
        __asm__ volatile ( "outb %%al, $0x80" : : "a"(0) );
    }
};
