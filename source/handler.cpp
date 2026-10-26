#include "handler.h"
#include "backtrace.h"
#include "report.h"
#include "screen.h"
#include "snapshot.h"
#include "utils/logger.h"
#include <coreinit/atomic.h>
#include <coreinit/context.h>
#include <coreinit/exception.h>
#include <coreinit/memorymap.h>
#include <coreinit/time.h>
#include <stdint.h>

extern "C" void PPCHalt(void);

static CrashSnapshot gSnapshot;
static char gReportLines[CRASH_REPORT_MAX_LINES][CRASH_REPORT_LINE_LEN];
static volatile uint32_t gInHandler           = 0;
static OSExceptionCallbackFn gPrevHandlers[4] = {NULL, NULL, NULL, NULL};

static const OSExceptionType gHandledTypes[4] = {
        OS_EXCEPTION_TYPE_DSI,
        OS_EXCEPTION_TYPE_ISI,
        OS_EXCEPTION_TYPE_ALIGNMENT,
        OS_EXCEPTION_TYPE_PROGRAM,
};

static int safe_read_u32(uint32_t addr, uint32_t *outValue) {
    if (!OSIsAddressValid(addr)) {
        return 0;
    }
    *outValue = *(volatile uint32_t *) addr;
    return 1;
}

static void log_report_lines(const char lines[][CRASH_REPORT_LINE_LEN], uint32_t count) {
#ifdef DEBUG
    for (uint32_t i = 0; i < count; i++) {
        WHBLogWritef("%s\n", lines[i]);
    }
#else
    for (uint32_t i = 0; i < count; i++) {
        OSReport("%s\n", lines[i]);
    }
#endif
}

static void spin_ticks(uint32_t ticks) {
    uint32_t start = OSGetSystemTick();
    while ((OSGetSystemTick() - start) < ticks) {
    }
}

static BOOL exception_cb(OSContext *ctx, uint32_t type) {
    if (!OSCompareAndSwapAtomic(&gInHandler, 0, 1)) {
        PPCHalt();
    }

    uint32_t lineCount = 0;
    if (ctx != NULL) {
        gSnapshot.exceptionType = type;
        for (uint32_t i = 0; i < 32; i++) {
            gSnapshot.gpr[i] = ctx->gpr[i];
        }
        gSnapshot.cr    = ctx->cr;
        gSnapshot.lr    = ctx->lr;
        gSnapshot.ctr   = ctx->ctr;
        gSnapshot.xer   = ctx->xer;
        gSnapshot.srr0  = ctx->srr0;
        gSnapshot.srr1  = ctx->srr1;
        gSnapshot.dsisr = ctx->dsisr;
        gSnapshot.dar   = ctx->dar;

        for (uint32_t i = 0; i < CRASH_SNAPSHOT_CODE_WORD_COUNT; i++) {
            uint32_t word = 0;
            if (!safe_read_u32(ctx->srr0 + 4 * i, &word)) {
                word = CRASH_SNAPSHOT_WORD_INVALID;
            }
            gSnapshot.codeWords[i] = word;
        }

        gSnapshot.stackDepth =
                crash_backtrace_walk(ctx->srr0, ctx->lr, ctx->gpr[1], safe_read_u32,
                                     gSnapshot.stackEntries, CRASH_SNAPSHOT_MAX_STACK_ENTRIES);

        lineCount = crash_report_build(&gSnapshot, gReportLines);
    } else {
        lineCount = crash_report_build_no_context(type, gReportLines);
    }

    log_report_lines(gReportLines, lineCount);

    spin_ticks(0x100000);
    crash_screen_show(gReportLines, lineCount);

    return FALSE;
}

static BOOL on_dsi(OSContext *ctx) {
    return exception_cb(ctx, OS_EXCEPTION_TYPE_DSI);
}

static BOOL on_isi(OSContext *ctx) {
    return exception_cb(ctx, OS_EXCEPTION_TYPE_ISI);
}

static BOOL on_alignment(OSContext *ctx) {
    return exception_cb(ctx, OS_EXCEPTION_TYPE_ALIGNMENT);
}

static BOOL on_program(OSContext *ctx) {
    return exception_cb(ctx, OS_EXCEPTION_TYPE_PROGRAM);
}

void handler_register_exception_callbacks() {
    static const OSExceptionCallbackFn trampolines[4] = {
            on_dsi,
            on_isi,
            on_alignment,
            on_program,
    };

    for (uint32_t i = 0; i < 4; i++) {
#ifdef EXCEPTION_MODE_GLOBAL_ALL_CORES
        gPrevHandlers[i] = OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES,
                                                    gHandledTypes[i], trampolines[i]);
#else
        gPrevHandlers[i] = OSSetExceptionCallback(gHandledTypes[i], trampolines[i]);
#endif
        DEBUG_FUNCTION_LINE_VERBOSE("registered type %u, prev %08X", (uint32_t) gHandledTypes[i],
                                    (uint32_t) gPrevHandlers[i]);
    }
}
