#pragma once

#include <stdint.h>

#define NO_STD_INT_HANDLERS

typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint32_t base;
} idtr;

typedef struct __attribute__((packed)) {
    uint16_t offset_lo;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_hi;
} idt_entry;

typedef struct  __attribute__ ((packed)) {
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    union {
        struct __attribute__ ((packed)) {
            uint32_t *esp;
            uint32_t ss;
        } user_to_kernel;
        uint32_t kernel_to_kernel_stack[0];
    };
} interrupt_frame;

#define IDT_ENTRY_COUNT 0x100

#define IRQ_COUNT 16

typedef struct {
    idtr reg;
    idt_entry entries[IDT_ENTRY_COUNT];
} idt_info;

typedef void (*raw_int_handler) (interrupt_frame *frame);
typedef void (*int_handler) (int interrupt, interrupt_frame *frame);
typedef void (*int_error_handler) (int interrupt, uint32_t &eip, uint32_t error);
typedef void (*irq_handler) (int irq, uint32_t &eip);
typedef void (*c8_handler) ();

enum struct handler_type { int_noerr, int_err, irq };

template <int Interrupt, handler_type Type> struct internal_handler;

struct interrupt {
private:    
    static int_handler m_ints[IDT_ENTRY_COUNT];
    static int_error_handler m_errs[IDT_ENTRY_COUNT];
    static irq_handler m_irqs[IRQ_COUNT];
    static bool m_irq_triggered[IRQ_COUNT];

    template <int IRQ> static void __attribute__ ((interrupt)) irq_handler_internal(void *unused);
    
    template <int Interrupt, handler_type Type> friend struct internal_handler;
public:
    static void init(idt_info *info);
    static void set_int(int interrupt, raw_int_handler handler, int rpl);
    static void map_irq(int irq, irq_handler handler);
    static void set_c8(c8_handler c8h);
    
    static void wait_irq(int irq);
};

inline int get_ring() {
    uint16_t ss;
    
    asm volatile ( "mov %%ss, %%ax\n"
                   "mov %%ax, %0\n": "=r"(ss) );
    
    return ss & 3;
}
