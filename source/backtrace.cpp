#include "backtrace.h"

uint32_t crash_backtrace_walk(uint32_t srr0, uint32_t lr, uint32_t stackPointer,
                              crash_read_u32_fn readFn, uint32_t *outEntries,
                              uint32_t maxEntries) {
    uint32_t count = 0;

    if (maxEntries == 0 || readFn == nullptr || outEntries == nullptr) {
        return 0;
    }

    outEntries[count++] = srr0;
    if (maxEntries == 1) {
        return count;
    }

    outEntries[count++] = lr;
    if (maxEntries == 2) {
        return count;
    }

    uint32_t frame = stackPointer;
    while (count < maxEntries) {
        if (frame == 0 || (frame & 3) != 0) {
            break;
        }

        uint32_t next = 0;
        uint32_t ret  = 0;
        if (!readFn(frame, &next) || !readFn(frame + 4, &ret)) {
            break;
        }

        if (next == 0) {
            break;
        }

        if (next < frame + 8 || (next & 3) != 0) {
            break;
        }

        outEntries[count++] = ret;
        frame               = next;
    }

    return count;
}
