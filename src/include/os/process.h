#pragma once

#include <cstdlib>
#include <cstring>

enum class x86_eflags : uint32_t {
    enable_interrupts = 0x200 
};

typedef int __cdecl (*proc_entry_point)(int argc, char **argv);

typedef struct __attribute__ ((packed)) {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t esp;
    uint32_t ebp;
    uint32_t eip;
    x86_eflags eflags;
} reg_values;

#define TASK_STACK_SIZE 0x1000

typedef struct __attribute__ ((packed)) {
    reg_values reg;
    
    uint8_t stack[TASK_STACK_SIZE];
} task;

class process {
private:
    task m_task;
    void init_task(proc_entry_point entry, bool ring3 = false) {
        memset(&m_task, 0, sizeof(task));
        m_task.reg.eip = reinterpret_cast<uint32_t>(entry);
        m_task.reg.esp = reinterpret_cast<uint32_t>(m_task.stack) + TASK_STACK_SIZE - 16; // 8
        m_task.reg.eflags = x86_eflags::enable_interrupts;
    }
public:
    process(proc_entry_point entry) {
        init_task(entry);
    }
    void run() {
        run_indir(m_task);
    }
    static void run_indir(task &task) {
        asm volatile ("\
            mov %[task], %%ebx      \n\
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
            iret"
            :: [task] "r" (&task)
        );
    }
};
