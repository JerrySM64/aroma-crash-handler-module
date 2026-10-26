#include "screen.h"
#include "report.h"
#include "utils/logger.h"
#include <coreinit/debug.h>

static char gScreenText[CRASH_SCREEN_TEXT_SIZE];

void crash_screen_show(const char lines[][CRASH_REPORT_LINE_LEN], uint32_t count) {
    crash_report_join(gScreenText, sizeof(gScreenText), lines, count);
    DEBUG_FUNCTION_LINE_VERBOSE("showing crash screen");
    OSFatal(gScreenText);
}
