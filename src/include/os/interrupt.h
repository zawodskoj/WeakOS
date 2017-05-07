#pragma once

#include <stdint.h>

typedef struct __attribute__ ((packed)) {
    uint16_t limit;
    uint32_t base;
} idtr;

typedef struct __attribute__ ((packed)) {
    uint16_t offset_lo;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_hi;
} idt_entry;

#define IDT_ENTRY_COUNT 0x100

#define IRQ_COUNT 16

typedef struct {
    idtr idtr;
    idt_entry entries[IDT_ENTRY_COUNT];
} idt_info;

typedef void (*int_handler) (int interrupt);
typedef void (*int_error_handler) (int interrupt, uint32_t error);
typedef void (*irq_handler) (int irq);

enum struct handler_type { int_noerr, int_err, irq };

template <int Interrupt, handler_type Type> struct internal_handler;

struct interrupt {
private:    
    static int_handler m_ints[IDT_ENTRY_COUNT];
    static int_error_handler m_errs[IDT_ENTRY_COUNT];
    static irq_handler m_irqs[IRQ_COUNT];

    template <int IRQ> static void __attribute__ ((interrupt)) irq_handler_internal(void *unused);
    
    template <int Interrupt, handler_type Type> friend class internal_handler;
public:
    static void init(idt_info *info);
    static void map(int interrupt, int_handler handler);
    static void map(int interrupt, int_error_handler handler);
    static void map_irq(int irq, irq_handler handler);
};
