#include "logger.h"

#ifdef DEBUG
#include <nsysnet/_socket.h>
#include <stdint.h>
#include <whb/log_cafe.h>
#include <whb/log_module.h>
#include <whb/log_udp.h>

static uint32_t moduleLogInit = false;
static uint32_t cafeLogInit   = false;
static uint32_t udpLogInit    = false;
#endif

void initLogging() {
#ifdef DEBUG
    socket_lib_init();
    moduleLogInit = WHBLogModuleInit();
    cafeLogInit   = WHBLogCafeInit();
    udpLogInit    = WHBLogUdpInit();
#endif
}

void deinitLogging() {
#ifdef DEBUG
    if (moduleLogInit) {
        WHBLogModuleDeinit();
        moduleLogInit = false;
    }
    if (cafeLogInit) {
        WHBLogCafeDeinit();
        cafeLogInit = false;
    }
    if (udpLogInit) {
        WHBLogUdpDeinit();
        udpLogInit = false;
    }
#endif
}
