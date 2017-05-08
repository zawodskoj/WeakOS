#include <cstdio>
#include <ustdio.h>

int printf ( const char * format, ... ) {
    stdio::printf(format);
    return 0;
}
