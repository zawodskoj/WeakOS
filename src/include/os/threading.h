#pragma once

#define NULL 0

typedef void *handle_t;
typedef handle_t process_t;

#define PROCESS_AUTOSTART 1

process_t create_process(const char *fileName, int flags);
void wait_handle(handle_t handle);
