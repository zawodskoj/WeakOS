#pragma once

#include <os/interrupt.h>

class time {
private:
    static volatile uint64_t m_ticks;
    
    static void irq0_handler(int irq, uint32_t) {
        m_ticks++;
    }
    
    static inline uint64_t rdtsc()
    {
        uint64_t ret;
        asm volatile ( "rdtsc" : "=A"(ret) );
        return ret;
    }
    
    static uint64_t m_rdtsc_rel;
    
public:
    static void init() {/*
        __asm__ volatile (
            "cli\n"
            "mov $0b00110100, %al\n"
            "outb %al, $0x43\n"
            "mov $0x80, %al\n"
            "outb %al, $0x40\n"
            "mov $0x0, %al\n"
            "outb %al, $0x40\n"
            "sti\n" );*/
        
        m_ticks = 0;
        interrupt::map_irq(0, irq0_handler);
        
/*        uint64_t rb = rdtsc();
        
        while (m_ticks < 0x10000);
        
        uint64_t ticks = m_ticks;
        uint64_t rticks = rdtsc() - rb;
        
        time::m_rdtsc_rel = rticks * 0x100000000 / ticks;*/
    }
    
    static uint64_t get_ticks() { return m_ticks; }
    static int get_seconds() { return m_ticks / 18; }
};
