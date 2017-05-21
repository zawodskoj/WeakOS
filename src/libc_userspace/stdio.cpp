#include <stdio.h>
#include <stdarg.h>
#include <os/syscall.h>

FILE *stdin = 0;

void put_int(int value, int &written) {
    if (value < 0) {
        putchar('-'); 
        written++; 
        value = -value; 
    }
    
    char str[20];
    int ln = 0;
    
    while (value > 0)  {
        str[ln++] = value % 10 + '0';
        value /= 10;
    }
    
    if (ln == 0) { 
        putchar('0');
        written++;
        return;
    } else for (int i = 1; i <= ln; i++, written++) putchar(str[ln - i]);
}

extern "C" {    
    int putchar (int ch) {
        uint32_t syscall_arg[1];
        syscall_arg[0] = (uint32_t) ch;
        syscall(1, 1, syscall_arg);
        return ch;
    }
    
    int printf (const char * format, ...) {
        int written = 0;
        
        va_list argl;
        va_start(argl, format);
        
        while (*format) {
            if (*format == '%') {
                switch (*(format + 1)) {
                    char *str;
                    int intv;
                    case '%':
                        ++format;
                        break;
                    case 'd':
                        intv = va_arg(argl, int);
                        put_int(intv, written);
                        format += 2;
                        break;
                    case 's':
                        str = va_arg(argl, char*);
                        while (*str) putchar(*str++), written++;
                        format += 2;
                        break;
                    default:
                        break; // invalid
                }
            }
            putchar(*format++);
            written++;
        }
        
        va_end(argl);
        return written;
    }
    
    int puts(const char *str) {
        while (*str) putchar(*str++);
        putchar('\n');
        return 1;
    }
    
    void perror(const char *str) {
        puts(str);
    }
    
    char * fgets ( char * str, int num, FILE * stream ) {
        uint32_t syscall_arg[1];
        syscall_arg[0] = (uint32_t) str;
        syscall(2, 1, syscall_arg);
        return str;
    }
}
