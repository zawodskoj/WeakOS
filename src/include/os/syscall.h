#pragma once

#include <ustdio.h>
#include <cstdlib>
#include <os/interrupt.h>
#include <os/process.h>
#include <os/fat.h>

typedef void (*voidfn)();

template <task &task, voidfn handler> static void __attribute__ ((naked)) save_context_and_go(interrupt_frame *) {
    asm volatile (
        "cli                    \n\
        push %%ebp              \n" /* Cохранение текущей задачи */ "\
        mov %%esp, %%ebp        \n\
        sub $20, %%esp          \n\
        mov %%eax, -4(%%ebp)    \n\
        mov %%ebx, -8(%%ebp)    \n\
        mov %%edx, -12(%%ebp)   \n\
        mov %[task], %%ebx     \n\
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
        mov %[handler], %%eax   \n\
        sti                     \n\
        jmpl *%%eax" :: [task] "i" (&task), [handler] "i" (handler));
}

const int syscall_interrupt = 0xc9;
const int syscall_max_cmd = 0x100;

enum class syscall_cmd : uint32_t {
    put_term = 1,
    get_term = 2,
    alloc = 3,
    dirent = 4
};

typedef uint32_t (*syscall_function)(void *stackframe);

class syscall {
private:
    static task m_task;
    static syscall_function m_cmds[syscall_max_cmd + 1];
    
    static void syscall_handler() {
        interrupt_frame *frame = reinterpret_cast<interrupt_frame*>(m_task.reg.esp - 12); // 12 - sizeof фрейма кернел-кернел
                                                                                          // костыль!
        
        uint32_t cmd = frame->cs & 3 ? frame->user_to_kernel.esp[0] : frame->kernel_to_kernel_stack[0];
        
        if (cmd <= syscall_max_cmd && cmd != 0 && m_cmds[cmd])
            frame->kernel_to_kernel_stack[0] = m_cmds[cmd](frame->kernel_to_kernel_stack + 1);
        
        process::run_indir(m_task);
    }
    
    static uint32_t syscall_put_term(void*);
    static uint32_t syscall_get_term(void*);
    static uint32_t syscall_alloc(void*);
    static uint32_t syscall_dirent(void*);
    
public:
    static fat *xfat;
    static void init() {
        // ЕСЛИ ВО ВРЕМЯ ОБРАБОТКИ ПРОИЗОЙДЕТ CONTEXT SWITCHING - БУДЕТ ПИЗДОС
        // ПОФИКСИТЬ БЛЯТБ!!!
        
        memset(m_cmds, 0, sizeof(m_cmds));
        m_cmds[(int) syscall_cmd::put_term] = syscall_put_term;
        m_cmds[(int) syscall_cmd::get_term] = syscall_get_term;
        m_cmds[(int) syscall_cmd::alloc] = syscall_alloc;
        m_cmds[(int) syscall_cmd::dirent] = syscall_dirent;
        
        interrupt::set_int(syscall_interrupt, save_context_and_go<m_task, syscall_handler>, 3);
        
        
    }
};
