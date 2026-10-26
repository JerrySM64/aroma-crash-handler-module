#pragma once

#include "report.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CRASH_SCREEN_TEXT_SIZE (CRASH_REPORT_MAX_LINES * CRASH_REPORT_LINE_LEN)

void crash_screen_show(const char lines[][CRASH_REPORT_LINE_LEN], uint32_t count);

#ifdef __cplusplus
}
#endif
