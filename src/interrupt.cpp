#include <os/interrupt.h>
#include <os/pic.h>
#include <os/io.h>
#include <ustdio.h>

int_handler interrupt::m_ints[IDT_ENTRY_COUNT];
int_error_handler interrupt::m_errs[IDT_ENTRY_COUNT];
irq_handler interrupt::m_irqs[IRQ_COUNT];
bool interrupt::m_irq_triggered[IRQ_COUNT];

template <int Interrupt, handler_type Type> struct internal_handler {
public:
    static void __attribute__ ((interrupt)) handler(uint32_t *unused) {
        stdio::printf("_int %d_ at %x\n", Interrupt, *unused);
        if (interrupt::m_ints[Interrupt]) interrupt::m_ints[Interrupt](Interrupt);
    }
};

template <> struct internal_handler<0xc8, handler_type::int_noerr> {
public:
    static void __attribute__ ((interrupt)) handler(uint32_t *unused) {
        
    }
};

template <int Interrupt> struct internal_handler<Interrupt, handler_type::irq> {
public:
    static void __attribute__ ((interrupt)) handler(uint32_t *unused) {
        if (Interrupt == 7 && !(pic_get_isr() & 0x80)) return;
        if (Interrupt == 15 && !(pic_get_isr() & 0x8000)) {
            io::outb(PIC1_COMMAND, PIC_EOI);
            return;
        }
        pic_eoi(Interrupt);
        interrupt::m_irq_triggered[Interrupt] = true;
        
        if (interrupt::m_irqs[Interrupt]) interrupt::m_irqs[Interrupt](Interrupt, *unused);
    }
};

template <int Interrupt> struct internal_handler<Interrupt, handler_type::int_err> {
public: 
    static void __attribute__ ((interrupt)) handler(void *unused, uint32_t error) {
        stdio::printf("_int %d_", Interrupt);
        if (interrupt::m_errs[Interrupt]) interrupt::m_errs[Interrupt](Interrupt, error);
    }
};

template <int Int, int Max, handler_type Type> struct int_filler {
public:
    static void fill(idt_entry *entry) {
        entry->offset_lo = reinterpret_cast<uint32_t>(internal_handler<Int, Type>::handler) & 0xffff;
        entry->offset_hi = (reinterpret_cast<uint32_t>(internal_handler<Int, Type>::handler) >> 16) & 0xffff;
        entry->selector = 8;
        entry->zero = 0;
        entry->type_attr = Int == 0xc8 ? 0xee : 0x8e;
        
        int_filler<Int + 1, Max, Type>::fill(entry + 1);
    }
};

template <int Max, handler_type Type> struct int_filler<Max, Max, Type> { public: static void fill(idt_entry *) {}; };

template <int From, int To, handler_type Type> struct fill_int {
public:
    static void go(idt_entry *entries) {
        int_filler<From, To + 1, Type>::fill(entries + From);
    }
};

template <int From, int To> struct fill_int<From, To, handler_type::irq> {
public:
    static void go(idt_entry *entries) {
        int_filler<From, To + 1, handler_type::irq>::fill(entries + 0x20 + From);
    }
};

void interrupt::init(idt_info *info) {
    pic_remap(0x20, 0x28);
    
    for (int i = 0; i < IRQ_COUNT; i++) { 
        interrupt::m_irqs[i] = 0;
    }
        
    info->reg.limit = IDT_ENTRY_COUNT * sizeof(idt_entry) - 1;
    info->reg.base = reinterpret_cast<uint32_t>(info->entries);
            
    fill_int<0, 7, handler_type::int_noerr>::go(info->entries);
    fill_int<8, 8, handler_type::int_err>::go(info->entries);
    fill_int<9, 9, handler_type::int_noerr>::go(info->entries);
    fill_int<10, 14, handler_type::int_err>::go(info->entries);
    fill_int<16, 16, handler_type::int_noerr>::go(info->entries);
    fill_int<17, 17, handler_type::int_err>::go(info->entries);
    fill_int<18, 19, handler_type::int_noerr>::go(info->entries);
    
    fill_int<0, 15, handler_type::irq>::go(info->entries);
    
    fill_int<0xc7, 0xc8, handler_type::int_noerr>::go(info->entries);
    
    __asm__ volatile ( "lidt %0\n\
                        in $0x70, %%al\n\
                        and $0x7f, %%al\n\
                        out %%al, $0x70\n\
                        sti\n" :: "m"(info->reg));
}

void interrupt::map_irq(int irq, irq_handler handler) {
    interrupt::m_irqs[irq] = handler;
}

void interrupt::wait_irq(int irq) {
    interrupt::m_irq_triggered[irq] = false;
    while (!interrupt::m_irq_triggered[irq]);
}
