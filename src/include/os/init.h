#pragma once

#include <os/paging.h>
#include <os/keyboard.h>
#include <os/interrupt.h>
#include <os/memory.h>
#include <os/ata.h>
#include <os/time.h>
#include <os/gdt.h>
#include <os/syscall.h>

#define KERNEL_RESERVED_BYTES 0x400000
#define KERNEL_STACK_SIZE 0x40000
#define KERNEL_ISR_STACK_SIZE 0x40000

typedef struct {
    uint8_t rsvd[KERNEL_RESERVED_BYTES];
    paging_info paging;
    idt_info idt;
    mem_info mem;
    gdt_info gdt;
    uint32_t stack[KERNEL_STACK_SIZE];
    uint32_t isr_stack[KERNEL_ISR_STACK_SIZE];
} kernel_data;

void sysinit();
