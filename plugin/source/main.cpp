#include "../../source/version.h"
#include <coreinit/debug.h>
#include <coreinit/dynload.h>
#include <wups.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/storage.h>

WUPS_PLUGIN_NAME("Crash Handler Config");
WUPS_PLUGIN_DESCRIPTION("Choose whether the crash screen shows the full stack dump.");
WUPS_PLUGIN_VERSION(CRASH_MODULE_VERSION);
WUPS_PLUGIN_AUTHOR("JerrySM64");
WUPS_PLUGIN_LICENSE("GPL-3.0");

WUPS_USE_STORAGE("crash_handler_config");

#define MODULE_NAME          "exception_handler"
#define MODULE_SET_FN_NAME   "CrashHandlerModule_SetShowStackDump"
#define SHOW_STACK_DUMP_KEY  "showStackDump"
#define SHOW_STACK_DUMP_DEFAULT false

typedef void (*SetShowStackDumpFn)(bool);

static bool sShowStackDump = SHOW_STACK_DUMP_DEFAULT;

static void apply_to_module() {
    OSDynLoad_Module module = nullptr;
    if (OSDynLoad_Acquire(MODULE_NAME, &module) != OS_DYNLOAD_OK) {
        OSReport("[CrashHandlerConfig] Module \"" MODULE_NAME "\" not found\n");
        return;
    }

    void *fn = nullptr;
    if (OSDynLoad_FindExport(module, OS_DYNLOAD_EXPORT_FUNC, MODULE_SET_FN_NAME, &fn) ==
                OS_DYNLOAD_OK &&
        fn != nullptr) {
        ((SetShowStackDumpFn) fn)(sShowStackDump);
    } else {
        OSReport("[CrashHandlerConfig] Export \"" MODULE_SET_FN_NAME "\" not found "
                 "(is the Crash Handler Module up to date?)\n");
    }

    OSDynLoad_Release(module);
}

static void show_stack_dump_changed(ConfigItemBoolean *, bool newValue) {
    sShowStackDump = newValue;
    WUPSStorageAPI::Store(SHOW_STACK_DUMP_KEY, sShowStackDump);
    apply_to_module();
}

static WUPSConfigAPICallbackStatus config_menu_opened(WUPSConfigCategoryHandle root) {
    if (WUPSConfigItemBoolean_AddToCategoryEx(root, SHOW_STACK_DUMP_KEY, "Show stack dump",
                                              SHOW_STACK_DUMP_DEFAULT, sShowStackDump,
                                              show_stack_dump_changed, "Yes", "No") !=
        WUPSCONFIG_API_RESULT_SUCCESS) {
        return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
    }
    return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;
}

static void config_menu_closed() {
    WUPSStorageAPI::SaveStorage();
}

INITIALIZE_PLUGIN() {
    WUPSConfigAPIOptionsV1 options = {.name = "Crash Handler"};
    WUPSConfigAPI_Init(options, config_menu_opened, config_menu_closed);

    WUPSStorageAPI::GetOrStoreDefault(SHOW_STACK_DUMP_KEY, sShowStackDump,
                                      SHOW_STACK_DUMP_DEFAULT);
    WUPSStorageAPI::SaveStorage();
    apply_to_module();
}

ON_APPLICATION_START() {
    apply_to_module();
}
