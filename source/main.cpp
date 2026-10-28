#include "handler.h"
#include "title.h"
#include "utils/logger.h"
#include "version.h"
#include <wums.h>

WUMS_MODULE_EXPORT_NAME("exception_handler");
WUMS_MODULE_DESCRIPTION("On-screen register, stack and code dump when an application crashes.");
WUMS_MODULE_VERSION(CRASH_MODULE_VERSION);
WUMS_MODULE_LICENSE("GPL-3.0");
WUMS_MODULE_AUTHOR("JerrySM64");

WUMS_INITIALIZE(args) {
    (void) args;
    initLogging();
    DEBUG_FUNCTION_LINE_INFO("ExceptionHandlerModule loaded");
}

WUMS_APPLICATION_STARTS() {
    initLogging();
    DEBUG_FUNCTION_LINE_INFO("Application started");
    handler_register_exception_callbacks();
    crash_title_lookup();
}

WUMS_APPLICATION_REQUESTS_EXIT() {
    DEBUG_FUNCTION_LINE_INFO("Application requests exit");
    deinitLogging();
}

WUMS_APPLICATION_ENDS() {
    crash_title_clear();
    LOG_EX_DEFAULT(OSReport, "## INFO## ", "\n", "Application ended");
}
