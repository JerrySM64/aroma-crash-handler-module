#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CRASH_EXCEPTION_TYPE_UNKNOWN     0xFFFFFFFFu

#define CRASH_SNAPSHOT_MAX_STACK_ENTRIES 16
#define CRASH_SNAPSHOT_CODE_WORD_COUNT   8
#define CRASH_SNAPSHOT_WORD_INVALID      0xFFFFFFFFu

typedef struct CrashSnapshot {
    uint32_t exceptionType;
    uint32_t gpr[32];
    uint32_t cr;
    uint32_t lr;
    uint32_t ctr;
    uint32_t xer;
    uint32_t srr0;
    uint32_t srr1;
    uint32_t dsisr;
    uint32_t dar;
    uint32_t codeWords[CRASH_SNAPSHOT_CODE_WORD_COUNT];
    uint32_t stackEntries[CRASH_SNAPSHOT_MAX_STACK_ENTRIES];
    uint32_t stackDepth;
} CrashSnapshot;

static inline const char *crash_exception_type_name(uint32_t type) {
    switch (type) {
        case 0:
            return "System Reset";
        case 1:
            return "Machine Check";
        case 2:
            return "DSI";
        case 3:
            return "ISI";
        case 4:
            return "External Interrupt";
        case 5:
            return "Alignment";
        case 6:
            return "Program";
        case 7:
            return "Floating Point";
        case 8:
            return "Decrementer";
        case 9:
            return "System Call";
        case 10:
            return "Trace";
        case 11:
            return "Performance Monitor";
        case 12:
            return "Breakpoint";
        case 13:
            return "System Interrupt";
        case 14:
            return "ICI";
        default:
            return "Unknown";
    }
}

#ifdef __cplusplus
}
#endif
