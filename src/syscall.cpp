#include <os/syscall.h>
#include <os/fat.h>

task syscall::m_task;
syscall_function syscall::m_cmds[syscall_max_cmd + 1];

typedef struct __attribute__ ((packed)) {
    uint32_t argc;
    uint32_t chr;
} put_term_frame;

typedef struct __attribute__ ((packed)) {
    uint32_t argc;
    char * pstr;
} get_term_frame;

typedef struct __attribute__ ((packed)) {
    uint32_t argc;
    uint32_t size;
} alloc_frame;

uint32_t syscall::syscall_put_term(void* pframe) {
    put_term_frame *frame = (put_term_frame*) pframe;
    stdio::putchar((char) frame->chr);
    return 0;
}

uint32_t syscall::syscall_get_term(void* pframe) {
    get_term_frame *frame = (get_term_frame*) pframe;
    stdio::getline(frame->pstr, 100);
    return 0;
}

uint32_t syscall::syscall_alloc(void* pframe) {
    alloc_frame *frame = (alloc_frame*) pframe;
    stdio::printf("allocating %x\n", frame->size);
    return (uint32_t) malloc(frame->size);
}

int offs = 0;

bool dirent_func(parsed_dir_entry *ent) {
    for (int i = 0; i < offs * 2; i++) stdio::putchar(' ');
    stdio::printf("entry %s\n", ent->actual_file_name); 
    if (ent->dirent.std.attributes & 0x10) {
        offs++;
        syscall::xfat->enumerate_directory(&ent->dirent, dirent_func);
        offs--;
    }
    return false;
}

uint32_t syscall::syscall_dirent(void* pframe) {
    stdio::printf("directory tree: \n");
    xfat->enumerate_directory(0, dirent_func);
    return 0;
}
