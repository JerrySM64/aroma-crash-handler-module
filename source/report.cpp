#include "report.h"
#include "snapshot.h"
#include "title.h"
#include <stdint.h>

static void append_hex8(char *dst, uint32_t value) {
    for (int i = 7; i >= 0; i--) {
        uint32_t digit = value & 0xF;
        dst[i]         = (char) (digit < 10 ? '0' + digit : 'A' + digit - 10);
        value >>= 4;
    }
}

static void append_dashes8(char *dst) {
    for (int i = 0; i < 8; i++) {
        dst[i] = '-';
    }
}

static char *append_str(char *dst, const char *src) {
    while (*src) {
        *dst++ = *src++;
    }
    return dst;
}

static char *append_hex8p(char *dst, uint32_t value) {
    append_hex8(dst, value);
    return dst + 8;
}

static char *append_dashes8p(char *dst) {
    append_dashes8(dst);
    return dst + 8;
}

static char *append_reg_cell(char *dst, const char *label, uint32_t value) {
    char *start = dst;
    dst         = append_str(dst, label);
    while (dst - start < 6) {
        *dst++ = ' ';
    }
    return append_hex8p(dst, value);
}

static void build_spec_row(char *dst, const char *label1, uint32_t value1, const char *label2,
                           uint32_t value2) {
    char *p = dst;
    p       = append_reg_cell(p, label1, value1);
    *p++    = ' ';
    p       = append_reg_cell(p, label2, value2);
    *p      = 0;
}

static void build_spec_row3(char *dst, const char *label1, uint32_t value1, const char *label2,
                            uint32_t value2, const char *label3, uint32_t value3) {
    char *p = dst;
    p       = append_reg_cell(p, label1, value1);
    *p++    = ' ';
    p       = append_reg_cell(p, label2, value2);
    *p++    = ' ';
    p       = append_reg_cell(p, label3, value3);
    *p      = 0;
}

static void build_line(char *dst, const char *text) {
    *append_str(dst, text) = 0;
}

uint32_t crash_report_build(const CrashSnapshot *snapshot,
                            char lines[CRASH_REPORT_MAX_LINES][CRASH_REPORT_LINE_LEN]) {
    if (snapshot == nullptr || lines == nullptr) {
        return 0;
    }

    uint32_t count = 0;
    char *p;

    p  = lines[count];
    p  = append_str(p, "Exception Crash: ");
    p  = append_str(p, crash_exception_type_name(snapshot->exceptionType));
    p  = append_str(p, " Exception!");
    *p = 0;
    count++;

    for (uint32_t row = 0; row < 8; row++) {
        p = lines[count];
        for (uint32_t col = 0; col < 4; col++) {
            uint32_t index = row + 8 * col;
            if (col > 0) {
                *p++ = ' ';
            }
            p    = append_str(p, "GPR");
            *p++ = (char) ('0' + index / 10);
            *p++ = (char) ('0' + index % 10);
            *p++ = ' ';
            p    = append_hex8p(p, snapshot->gpr[index]);
        }
        *p = 0;
        count++;
    }

    build_spec_row3(lines[count], "LR", snapshot->lr, "CTR", snapshot->ctr, "XER",
                    snapshot->xer);
    count++;
    build_spec_row(lines[count], "SRR0", snapshot->srr0, "SRR1", snapshot->srr1);
    count++;
    build_spec_row(lines[count], "DAR", snapshot->dar, "DSISR", snapshot->dsisr);
    count++;

    build_line(lines[count], "");
    count++;

    build_line(lines[count], "STACK DUMP:");
    count++;

    for (uint32_t i = 0; i < snapshot->stackDepth; i += 4) {
        p = lines[count];
        for (uint32_t j = i; j < i + 4 && j < snapshot->stackDepth; j++) {
            if (j > i) {
                p = append_str(p, " --> ");
            }
            p = append_hex8p(p, snapshot->stackEntries[j]);
        }
        *p = 0;
        count++;
    }

    build_line(lines[count], "");
    count++;

    build_line(lines[count], "CODE DUMP:");
    count++;

    for (uint32_t line = 0; line < 2; line++) {
        p    = lines[count];
        p    = append_hex8p(p, snapshot->srr0 + 0x10 * line);
        *p++ = ':';
        *p++ = ' ';
        *p++ = ' ';
        for (uint32_t w = 0; w < 4; w++) {
            uint32_t word = snapshot->codeWords[line * 4 + w];
            if (w > 0) {
                *p++ = ' ';
            }
            if (word == CRASH_SNAPSHOT_WORD_INVALID) {
                p = append_dashes8p(p);
            } else {
                p = append_hex8p(p, word);
            }
        }
        *p = 0;
        count++;
    }

    build_line(lines[count], "");
    count++;

    p = lines[count];
    p = append_str(p, "- ");
    p = append_str(p, crash_title_get_name());
    *p = 0;
    count++;

    return count;
}

uint32_t crash_report_build_no_context(uint32_t exceptionType,
                                       char lines[CRASH_REPORT_MAX_LINES][CRASH_REPORT_LINE_LEN]) {
    if (lines == nullptr) {
        return 0;
    }

    char *p = lines[0];
    p       = append_str(p, "Exception Crash: ");
    p       = append_str(p, crash_exception_type_name(exceptionType));
    p       = append_str(p, " Exception! (no context)");
    *p      = 0;
    p       = lines[1];
    p       = append_str(p, "- ");
    p       = append_str(p, crash_title_get_name());
    *p      = 0;
    return 2;
}

uint32_t crash_report_join(char *out, uint32_t outSize,
                           const char lines[][CRASH_REPORT_LINE_LEN], uint32_t count) {
    if (out == nullptr || outSize == 0) {
        return 0;
    }

    uint32_t pos = 0;
    for (uint32_t i = 0; i < count; i++) {
        const char *src = lines[i];
        const char *end = src + CRASH_REPORT_LINE_LEN;
        for (; src != end && *src != '\0'; src++) {
            if (pos >= outSize - 1) {
                break;
            }
            out[pos++] = *src;
        }
        if (pos >= outSize - 1) {
            break;
        }
        if (i + 1 < count) {
            out[pos++] = '\n';
        }
    }
    out[pos] = '\0';
    return pos;
}
