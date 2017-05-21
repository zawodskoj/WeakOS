#include <cstddef>

extern "C" {
    size_t strlen(const char *s) {
        size_t len = 0;
        while (*s++) len++;
        return len;
    }
    
    int strcmp(const char *lhs, const char *rhs) {
        while (*lhs) {
            if (!*rhs) return 1;
            if (*lhs < *rhs) return -1;
            if (*lhs > *rhs) return 1;
            lhs++, rhs++;
        }
        return *rhs ? -1 : 0;
    }
    
    char* strcpy(char *dest, const char *src) {
        while (*src) *dest++ = *src++;
        *dest = 0;
        return dest;            
    }
}
