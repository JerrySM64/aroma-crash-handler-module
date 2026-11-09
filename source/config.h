#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool crash_config_get_show_stack_dump(void);
void crash_config_set_show_stack_dump(bool show);

#ifdef __cplusplus
}
#endif
