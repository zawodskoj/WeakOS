#pragma once

#include <cstdint>

uint32_t syscall(uint32_t syscall_num, int argc, uint32_t *argv);
