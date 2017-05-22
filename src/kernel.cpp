#include <ustdio.h>

#include <os/fat.h>
#include <os/elf.h>
#include <os/process.h>
#include <os/syscall.h>
#include <os/init.h>

#include <string>

fat *syscall::xfat;

int main() {
    sysinit();
    stdio::printf("libc++ testing\ntrying to create std::string...");
    std::string kek = "Hello!";
    stdio::printf("done!\ntrying to print...");
    stdio::printf(" \"%s\" - done!!!", kek.c_str());
    while (1);
    
    /*
    syscall::xfat = new fat();
    uint32_t size = syscall::xfat->get_file_size("test.elf");
    stdio::printf("elf file size: %u\n", size);
    uint8_t *elf_data = new uint8_t[size];    
    syscall::xfat->get_file("testdir/test.elf", elf_data);
    stdio::printf("loaded!\n");
    
    elf *v_elf = new elf(elf_data);
    process *p = new process(v_elf->m_entry);
    p->run();
    
    //v_elf->run();
    stdio::printf("done!\n");*/
}

void *__gcc_personality_v0;

typedef enum {
	_URC_NO_REASON = 0,
	_URC_FOREIGN_EXCEPTION_CAUGHT = 1,
	_URC_FATAL_PHASE2_ERROR = 2,
	_URC_FATAL_PHASE1_ERROR = 3,
	_URC_NORMAL_STOP = 4,
	_URC_END_OF_STACK = 5,
	_URC_HANDLER_FOUND = 6,
	_URC_INSTALL_CONTEXT = 7,
	_URC_CONTINUE_UNWIND = 8
} _Unwind_Reason_Code;

typedef enum {
        _UA_SEARCH_PHASE = 1,
        _UA_CLEANUP_PHASE = 2,
        _UA_HANDLER_FRAME = 4,
        _UA_FORCE_UNWIND = 8,
        _UA_END_OF_STACK = 16	// gcc extension to C++ ABI
} _Unwind_Action;


struct _Unwind_Context;		// opaque 
struct _Unwind_Exception;	// forward declaration

struct _Unwind_Exception {
	uint64_t                   exception_class;
	void					 (*exception_cleanup)(_Unwind_Reason_Code reason, struct _Unwind_Exception* exc);
	uintptr_t                  private_1;        // non-zero means forced unwind
	uintptr_t                  private_2;        // holds sp that phase1 found for phase2 to use
#ifndef __LP64__
	// The gcc implementation of _Unwind_Exception used attribute mode on the above fields
	// which had the side effect of causing this whole struct to round up to 32 bytes in size.
	// To be more explicit, we add pad fields added for binary compatibility.  
	uint32_t				reserved[3];
#endif
};

extern "C" void _Unwind_Resume(struct _Unwind_Exception *object) {
    while (1);
}

extern "C" _Unwind_Reason_Code _Unwind_RaiseException(struct _Unwind_Exception* exception_object) {
    while (1);
}

typedef _Unwind_Reason_Code (*_Unwind_Trace_Fn)(struct _Unwind_Context*, void*);
extern "C" _Unwind_Reason_Code	_Unwind_Backtrace(_Unwind_Trace_Fn, void*) {
}

extern "C" uintptr_t _Unwind_GetIP(struct _Unwind_Context* context) { return 0; }
extern "C" uintptr_t _Unwind_GetCFA(struct _Unwind_Context* context) { return 0; }
extern "C" uintptr_t _Unwind_GetGR(struct _Unwind_Context* context, int index) { return 0; }

extern "C" void		_Unwind_SetIP(struct _Unwind_Context*, uintptr_t new_value) {}
extern "C" void		_Unwind_SetCFA(struct _Unwind_Context*, uintptr_t new_value) {}
extern "C" void		_Unwind_SetGR(struct _Unwind_Context*, int index, uintptr_t new_value) {}
extern "C" uintptr_t _Unwind_GetRegionStart(struct _Unwind_Context* context) {return 0;}
extern "C" uintptr_t _Unwind_GetLanguageSpecificData(struct _Unwind_Context* context) {return 0;}
extern "C" void		_Unwind_DeleteException(struct _Unwind_Exception* exception_object) {}


extern "C" {
    int   pthread_mutex_lock(void *) {return 0;}
    int   pthread_mutex_unlock(void *) {return 0;}
    int   pthread_setspecific(int, const void *) {return 0;}
    void*   pthread_getspecific(int) {return 0;}
    int   pthread_once(void*, void (*)(void)) {return 0;}
    int   pthread_key_create(void *, void (*)(void *)) {return 0;}
}
