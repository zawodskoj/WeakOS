#pragma once

#include <stdint.h>
#include <stddef.h>
#include <cpp.h>
#include <os/keyboard.h>
#include <os/io.h>

struct textmode_cell {
private:
    char m_ch;
    uint8_t m_attr;
public:
    char get_char() { return m_ch; }
    int get_attr() { return m_attr; }
    void set(char ch, int attr) {
        m_ch = ch;
        m_attr = attr;
    }
};

struct textmode_buffer {
private:
    static textmode_cell * m_buffer;
    static int m_width, m_height;
public:
    static void init(int width, int height) {
        m_buffer = reinterpret_cast<textmode_cell*>(0xb8000);
        m_width = width;
        m_height = height;
        
        // TODO: memset
        
        for (int i = 0; i < width * height; i++) m_buffer[i].set(' ', 7);
    }
    
    static textmode_cell* cell(int x, int y) {
        return &m_buffer[x + m_width * y];
    }
    
    static void scroll() {
        for (int i = 0; i < textmode_buffer::m_width * (textmode_buffer::m_height - 1); i++) 
            m_buffer[i] = m_buffer[i + textmode_buffer::m_width];
    
        for (int i = 0; i < textmode_buffer::m_width; i++) 
            m_buffer[i + textmode_buffer::m_width * (textmode_buffer::m_height - 1)].set(' ', 7);
    }
    
    static void refresh() {}
};

struct stdio {
private:
    static int m_deferred;
    
    struct defer_refresh {
    public:
        defer_refresh() { stdio::m_deferred++; }
#ifdef SLOWDOWN
        ~defer_refresh() { stdio::m_deferred--; if (!stdio::m_deferred) for (int i = 0; i < 0x100000; i++) io::wait(); }
#else
        ~defer_refresh() { stdio::m_deferred--; }
#endif
    };
    
    static int m_cx, m_cy, m_width, m_height, m_attr;
    
    static bool process_control(char control);
    template <typename T> static void putval(const char **format, T value);
    
public:    
    static void init();
    static void putchar(char ch);
    static void printf(const char *string);
    static void getline(char *output, size_t size);
    static void scanf(char *output, size_t size, const char *format);
    template <typename T, typename... Args>
        static void printf(const char *format, T value, Args... args);
    template <typename T, typename... Args>
        static void scanf(char *output, size_t size, const char *format, T value, Args... args);
};

template <> inline void stdio::putval<uint64_t>(const char **format, uint64_t value) {
    switch (**format) {
        case 'i':
        case 'u':
        {
            char str[20];
            int ln = 0;
            
            while (value > 0)  {
                str[ln++] = value % 10 + '0';
                value /= 10;
            }
            
            if (ln == 0) stdio::putchar('0');
            else for (int i = 1; i <= ln; i++) stdio::putchar(str[ln - i]);
            
            (*format)++;
            break;
        }
        case 'X':
        case 'x':
        case 'p':
        {
            const char *hexAlphaLow = "0123456789abcdef";
            const char *hexAlphaUp = "0123456789ABCDEF";
            
            const char *hexAlpha = **format == 'x' ? hexAlphaLow : hexAlphaUp;

            char str[16];
            int ln = 0;
            
            while (value > 0)  {
                str[ln++] = hexAlpha[value & 0xf];
                value >>= 4;
            }
            
            if (ln == 0) stdio::putchar('0');
            else for (int i = 1; i <= ln; i++) stdio::putchar(str[ln - i]);
            
            (*format)++;
            break;
        }       
    }
}
    
template <> inline void stdio::putval<unsigned int>(const char **format, unsigned int value) {
    putval(format, (uint64_t) value);
}

template <> inline void stdio::putval<int64_t>(const char **format, int64_t value) {
    switch (**format) {
        case 'i':
        case 'd':
        {
            if (value < 0) { 
                stdio::putchar('-'); 
                value = -value; 
            }
            
            char str[20];
            int ln = 0;
            
            while (value > 0)  {
                str[ln++] = value % 10 + '0';
                value /= 10;
            }
            
            if (ln == 0) stdio::putchar('0');
            else for (int i = 1; i <= ln; i++) stdio::putchar(str[ln - i]);
            
            (*format)++;
            break;
        }
        case 'X':
        case 'x':
            stdio::putval(format, (uint64_t) value);            
    }
}
    
template <> inline void stdio::putval<int>(const char **format, int value) {
    putval(format, (int64_t) value);
}

template <> inline void stdio::putval<long>(const char **format, long value) {
    putval(format, (int64_t) value);
}

template <> inline void stdio::putval<unsigned long>(const char **format, unsigned long value) {
    putval(format, (uint64_t) value);
}

template <> inline void stdio::putval<void*>(const char **format, void* value) {
    putval(format, (uint64_t) value);
}

template <> inline void stdio::putval<const char*>(const char **format, const char *value) {
    if (**format != 's') return;
    (*format)++;
    
    while (*value) stdio::putchar(*value++);
}

template <> inline void stdio::putval<char*>(const char **format, char *value) {
    stdio::putval(format, const_cast<const char*>(value));
}

template <> inline void stdio::putval<unsigned char*>(const char **format, unsigned char *value) {
    stdio::putval(format, const_cast<const char*>(reinterpret_cast<char*>(value)));
}

template <> inline void stdio::putval<unsigned char>(const char **format, unsigned char value) {
    stdio::putval(format, static_cast<int64_t>(value));
}

template <typename T, typename... Args> inline void stdio::printf(const char *format, T value, Args... args) {
    stdio::defer_refresh _;
    
    while (*format) {
        if (*format == '%') {
            if (*(format + 1) == '%') {
                ++format;
            }
            else {
                stdio::putval(&++format, value);
                printf(format, args...);
                return;
            }
        }
        stdio::putchar(*format++);
    }    
}
