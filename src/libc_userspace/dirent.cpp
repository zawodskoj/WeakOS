#include <dirent.h>
#include <cstring>
#include <os/syscall.h>

static int temp;
static struct dirent ent;

extern "C" {
    int closedir(DIR *dirp) { return 0; }
    
    DIR* opendir(const char *name) { temp = 0; return (DIR*)1; }
    
    struct dirent* readdir(DIR *dirp) {         
        if (temp == 0) {
            temp = 1;
            ent.d_ino = 123;
            ent.d_name[0] = 0;
            
            syscall(4, 0, 0);
            
            return &ent;
        } else return 0;
    }
    
    int readdir_r(DIR *dirp, struct dirent* entry, struct dirent** result) {
        return 0;
    }
    
    void rewinddir(DIR *dirp) {}
    
    void seekdir(DIR *dirp, long int loc) {}
    
    long int telldir(DIR *dirp) { return 0; }
}
