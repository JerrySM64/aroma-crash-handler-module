#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*crash_read_u32_fn)(uint32_t addr, uint32_t *outValue);

uint32_t crash_backtrace_walk(uint32_t srr0, uint32_t lr, uint32_t stackPointer,
                              crash_read_u32_fn readFn, uint32_t *outEntries,
                              uint32_t maxEntries);

#ifdef __cplusplus
}
#endif
