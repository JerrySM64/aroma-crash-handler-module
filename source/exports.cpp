#include "config.h"
#include "utils/logger.h"
#include <wums.h>

extern "C" void CrashHandlerModule_SetShowStackDump(bool show) {
    DEBUG_FUNCTION_LINE_INFO("Show stack dump: %d", (int) show);
    crash_config_set_show_stack_dump(show);
}

extern "C" bool CrashHandlerModule_GetShowStackDump() {
    return crash_config_get_show_stack_dump();
}

WUMS_EXPORT_FUNCTION(CrashHandlerModule_SetShowStackDump);
WUMS_EXPORT_FUNCTION(CrashHandlerModule_GetShowStackDump);
