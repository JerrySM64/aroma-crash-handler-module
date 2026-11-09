#include "config.h"

static volatile bool gShowStackDump = false;

bool crash_config_get_show_stack_dump(void) {
    return gShowStackDump;
}

void crash_config_set_show_stack_dump(bool show) {
    gShowStackDump = show;
}
