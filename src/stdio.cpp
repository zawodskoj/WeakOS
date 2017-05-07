#include <stdint.h>
#include <stdio.h>

#include <os/keyboard.h>

#include <cpp.h>

int stdio::m_cx = 0;
int stdio::m_cy = 0;
int stdio::m_width = 0;
int stdio::m_height = 0;
int stdio::m_attr = 0;
int stdio::m_deferred = false;

int textmode_buffer::m_width;
int textmode_buffer::m_height;
textmode_cell* textmode_buffer::m_buffer;

void stdio::init() {
    const int w = 80;
    const int h = 25;
    
    stdio::m_deferred = 0;
    stdio::m_cx = stdio::m_cy = 0;
    stdio::m_width = w;
    stdio::m_height = h;
    
    stdio::m_attr = 7;
    
    textmode_buffer::init(w, h);
}

void stdio::printf(const char *s) {
    stdio::defer_refresh _;
    
    while (*s) {
        if (*s == '%') {
            if (*(s + 1) == '%') {
                ++s;
            }
            else; /* TODO: exception handling */
        }
        stdio::putchar(*s++);
    }
}

bool stdio::process_control(char ch) {
    switch (ch) {
        case '\n':
            stdio::m_cy++;
            stdio::m_cx = 0;
            break;
        case '\t':
            stdio::m_cx += 4;
            break;
        default:
            return false;
    }    
    if (stdio::m_cx >= stdio::m_width) {
        stdio::m_cy++;
        stdio::m_cx = 0;
    }
    
    if (stdio::m_cy >= stdio::m_height) {
        textmode_buffer::scroll();
        stdio::m_cy--;
        // пока ничего не делаем, будем интегрировать скроллинг
    }
    return true;
}

void stdio::putchar(char ch) {
    if (stdio::process_control(ch)) return;
    
    textmode_buffer::cell(stdio::m_cx++, stdio::m_cy)->set(ch, stdio::m_attr);
    
    if (stdio::m_cx >= stdio::m_width) {
        stdio::m_cy++;
        stdio::m_cx = 0;
    }
    
    if (stdio::m_cy >= stdio::m_height) {
        textmode_buffer::scroll();
        stdio::m_cy--;
        // пока ничего не делаем, будем интегрировать скроллинг
    }
    
    textmode_buffer::refresh();
}

void stdio::getline(char *ch, size_t size) {
    size_t i = 0;
    while (i < size - 1) {
        char c = keyboard::wait_char();
        if (c == '\n') {
            ch[i] = 0;
            return;
        }
        ch[i++] = c;
    }
    ch[i] = 0;
}
