#include <cstdint>
#include <cstddef>

extern "C" {    
    void* memset(void *ptr, int v, size_t size) {
        uint8_t value = v;
        
        uint8_t *cptr = static_cast<uint8_t*>(ptr);
        
        if (size < 4) {
            for (; size > 0; size--) *cptr++ = value;
            return ptr;
        }
        
        uint32_t dvalue = value | (value << 8) | (value << 16) | (value << 24);
        
        for (; !(reinterpret_cast<uint32_t>(cptr) & 3); size--) *cptr++ = value;
        
        uint32_t *dptr = reinterpret_cast<uint32_t*>(cptr);
        for (; size > 4; size -= 4) *dptr++ = dvalue;
        
        cptr = reinterpret_cast<uint8_t*>(dptr);
        
        for (; size > 0; cptr++, size--) *cptr = value;
        
        return ptr;
    }
    
    extern "C" void* memcpy(void *dest, const void *src, size_t num) {
        auto cdest = reinterpret_cast<uint8_t*>(dest);
        auto csrc = reinterpret_cast<const uint8_t*>(src);
        
        for (size_t i = 0; i < num; i++) cdest[i] = csrc[i];
        return dest;
    }
}
