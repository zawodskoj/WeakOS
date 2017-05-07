#pragma once

#include <stdint.h>

extern "C" uint64_t __udivdi3(uint64_t a, uint64_t b);
extern "C" uint64_t __umoddi3(uint64_t a, uint64_t b);
extern "C" int64_t __divdi3(int64_t a, int64_t b);
extern "C" int64_t __moddi3(int64_t a, int64_t b);
