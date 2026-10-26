#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CRASH_REPORT_MAX_LINES 32
#define CRASH_REPORT_LINE_LEN  64

struct CrashSnapshot;

uint32_t crash_report_build(const CrashSnapshot *snapshot,
                            char lines[CRASH_REPORT_MAX_LINES][CRASH_REPORT_LINE_LEN]);

uint32_t crash_report_build_no_context(uint32_t exceptionType,
                                       char lines[CRASH_REPORT_MAX_LINES][CRASH_REPORT_LINE_LEN]);

uint32_t crash_report_join(char *out, uint32_t outSize,
                           const char lines[][CRASH_REPORT_LINE_LEN], uint32_t count);

#ifdef __cplusplus
}
#endif
