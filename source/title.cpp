#include "title.h"
#include "report.h"
#include "utils/logger.h"
#include <coreinit/title.h>
#include <malloc.h>
#include <nn/acp/client.h>
#include <nn/acp/title.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CRASH_TITLE_NAME_BUF_LEN 96
#define CRASH_TITLE_NAME_MAX_LEN (CRASH_REPORT_LINE_LEN - 3)
#define CRASH_TITLE_NAME_UNKNOWN "Unknown"

static char gTitleName[CRASH_TITLE_NAME_BUF_LEN];

void crash_title_clear(void) {
    gTitleName[0] = '\0';
}

const char *crash_title_get_name(void) {
    return gTitleName;
}

static uint32_t sanitize_and_store(const char *src, uint32_t srcSize) {
    uint32_t out = 0;
    uint32_t pendingSpace = 0;

    for (uint32_t i = 0; i < srcSize && src[i] != '\0'; i++) {
        char c = src[i];

        if (c == '\n' || c == '\r' || c == '\t') {
            c = ' ';
        }
        if (c == ' ') {
            pendingSpace = out > 0;
            continue;
        }
        if ((unsigned char) c < 0x20 || (unsigned char) c > 0x7E) {
            continue;
        }
        if (out >= CRASH_TITLE_NAME_MAX_LEN) {
            break;
        }
        if (pendingSpace) {
            gTitleName[out++] = ' ';
            pendingSpace      = 0;
        }
        gTitleName[out++] = c;
    }

    gTitleName[out] = '\0';
    return out;
}

static void store_title_id_fallback(uint64_t titleId) {
    snprintf(gTitleName, sizeof(gTitleName), "%016llX", (unsigned long long) titleId);
}

void crash_title_lookup(void) {
    crash_title_clear();

    uint64_t titleId = 0;

    ACPInitialize();

    ACPTitleId mainAppTitleId = 0;
    if (ACPGetTitleIdOfMainApplication(&mainAppTitleId) == ACP_RESULT_SUCCESS &&
        mainAppTitleId != 0) {
        titleId = (uint64_t) mainAppTitleId;
    } else {
        titleId = OSGetTitleID();
    }

    if (titleId != 0) {
        ACPMetaXml *meta = (ACPMetaXml *) memalign(0x40, sizeof(ACPMetaXml));
        if (meta != nullptr) {
            if (ACPGetTitleMetaXml((ACPTitleId) titleId, meta) == ACP_RESULT_SUCCESS) {
                if (sanitize_and_store(meta->longname_en, sizeof(meta->longname_en)) == 0) {
                    sanitize_and_store(meta->shortname_en, sizeof(meta->shortname_en));
                }
            }
            free(meta);
        }
    }

    ACPFinalize();

    if (gTitleName[0] == '\0') {
        if (titleId != 0) {
            store_title_id_fallback(titleId);
        } else {
            sanitize_and_store(CRASH_TITLE_NAME_UNKNOWN, sizeof(CRASH_TITLE_NAME_UNKNOWN));
        }
    }

    DEBUG_FUNCTION_LINE_INFO("Resolved crash screen title name: \"%s\" (title ID: %016llX)",
                             gTitleName, (unsigned long long) titleId);
}
