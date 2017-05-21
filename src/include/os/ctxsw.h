// TODO разобраться в этой хуйне, мне впадлу пока

#include <ustdio.h>

#include <os/ata.h>
#include <os/time.h>
#include <os/interrupt.h>

volatile uint64_t time::m_ticks;

/*static inline void switch_usermode() {
    asm volatile ( 
        "mov $0x23, %ax\n" 
        "mov %ax, %ds\n" 
        "mov %ax, %es\n" 
        "mov %esp, %eax\n" 
        "push $0x23\n" 
        "push %eax\n"
        "pushf\n"
        "push $0x1b\n"
        "push $1f\n"
        "iret\n"
        "1: \n" );
}*/

#define TASK_STACK_SIZE 0x1000

typedef struct {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t esp;
    uint32_t ebp;
    uint32_t eip;
    uint32_t eflags;
} reg_values;

typedef struct {
    reg_values reg;
    
    uint8_t stack[TASK_STACK_SIZE];
} task;

static task tasks[4];

static int task_cnt = 4;

static int cur_task = 0;

void __attribute__ ((naked)) c8_handler_i() {
    asm volatile (
        "cli                    \n\
        push %%ebp              \n" /* Cохранение текущей задачи */ "\
        mov %%esp, %%ebp        \n\
        sub $20, %%esp          \n\
        mov %%eax, -4(%%ebp)    \n\
        mov %%ebx, -8(%%ebp)    \n\
        mov %%edx, -12(%%ebp)   \n\
        mov %[tasks], %%ebx     \n\
        mov %[cur_task], %%eax  \n\
        mov %[t_sz], %%edx      \n\
        mov (%%eax), %%eax      \n\
        mul %%edx               \n\
        add %%eax, %%ebx        \n\
        mov -4(%%ebp), %%eax    \n\
        mov %%eax, (%%ebx)      \n\
        mov -8(%%ebp), %%eax    \n\
        mov %%eax, 4(%%ebx)     \n\
        mov %%ecx, 8(%%ebx)     \n\
        mov -12(%%ebp), %%eax   \n\
        mov %%eax, 12(%%ebx)    \n\
        mov %%esi, 16(%%ebx)    \n\
        mov %%edi, 20(%%ebx)    \n\
        mov 8(%%ebp), %%eax     \n\
        and $3, %%eax           \n\
        test %%eax, %%eax       \n\
        jz 1f                   \n\
        mov 16(%%ebp), %%eax    \n\
        jmp 2f                  \n\
        1: mov %%ebp, %%eax     \n\
        add $16, %%eax          \n\
        2: mov %%eax, 24(%%ebx) \n\
        mov (%%ebp), %%eax      \n\
        mov %%eax, 28(%%ebx)    \n\
        mov 4(%%ebp), %%eax     \n\
        mov %%eax, 32(%%ebx)    \n\
        mov 12(%%ebp), %%eax    \n\
        mov %%eax, 36(%%ebx)    \n\
        \
        " /* Загрузка следующей задачи */ "\
        mov %[cur_task], %%ecx  \n\
        mov (%%ecx), %%eax      \n\
        inc %%eax               \n\
        xor %%edx, %%edx        \n\
        mov %[t_count], %%esi   \n\
        mov (%%esi), %%esi      \n\
        div %%esi               \n\
        mov %%edx, (%%ecx)      \n\
        mov %[tasks], %%ebx     \n\
        mov %[t_sz], %%eax      \n\
        xchg %%edx, %%eax       \n\
        mul %%edx               \n\
        add %%eax, %%ebx        \n\
        mov 8(%%ebx), %%ecx     \n\
        mov 12(%%ebx), %%edx    \n\
        mov 16(%%ebx), %%esi    \n\
        mov 20(%%ebx), %%edi    \n\
        mov 24(%%ebx), %%esp    \n\
        mov 28(%%ebx), %%ebp    \n\
        mov 36(%%ebx), %%eax    \n\
        push %%eax              \n\
        mov 32(%%ebx), %%eax    \n\
        push $8                 \n\
        push %%eax              \n\
        mov (%%ebx), %%eax      \n\
        mov 4(%%ebx), %%ebx     \n\
        " /* sti не имеет значения, eflags переустановится в iret */ "\
        " /* Возврат */ "\
        iret"
        :: [tasks] "i" (tasks), [cur_task] "i" (&cur_task),
           [t_sz] "i" (sizeof(task)), [t_count] "i" (&task_cnt)
    );
}

void __attribute__ ((naked)) yield() { asm volatile ( "int $0xc8\nret\n" ); }

void irq0_handler_i(int, uint32_t&) {
    yield();
}

template <int Task> void test_task() { 
    if (Task == 0)
        interrupt::map_irq(0, irq0_handler_i);
    
    const int clrofs = 1 + 63 * Task;
    int clr = 0;
    while (1) {
        for (int i = Task * 50; i < (Task + 1) * 50; i++) {
            for (int j = 0; j < 320; j++) {
                *(uint8_t*) (0xa0000 + j + i * 320) = clr + clrofs;
                for (int sd = 0; sd < 0x100; sd++) io::wait();
            }
        }
        // *(uint16_t*) (0xb8000) = (Task + '0') | 0x700;
        clr = (clr + 1) % 64;
    } 
}

void mk_task(task &t, void (*fn)()) {
    t.reg.esp = reinterpret_cast<uint32_t>(t.stack + TASK_STACK_SIZE);
    t.reg.eip = reinterpret_cast<uint32_t>(fn);
    t.reg.eflags = 0x200; // carry flag
}

int main() {
    interrupt::set_c8(c8_handler_i);
    
    mk_task(tasks[0], test_task<0>);
    mk_task(tasks[1], test_task<1>);
    mk_task(tasks[2], test_task<2>);
    mk_task(tasks[3], test_task<3>);
        
    test_task<0>(); // Надо запускать с нормальным стеком
}
