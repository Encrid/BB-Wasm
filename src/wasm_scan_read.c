/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "wasm_scan.h"

static uint64_t leb(scan_t *s, uint32_t *shift, uint8_t *last)
{
    uint64_t value = 0;
    uint8_t byte = 0x80;

    *shift = 0;
    while (!s->bad && (byte & 0x80)) {
        if (s->at >= s->end || *shift > 63) {
            s->bad = true;
            return (0);
        }
        byte = *s->at++;
        value |= (uint64_t)(byte & 0x7F) << *shift;
        *shift += 7;
    }
    *last = byte;
    return (value);
}

uint64_t scan_u64(scan_t *s)
{
    uint32_t shift;
    uint8_t last;

    return (leb(s, &shift, &last));
}

uint32_t scan_u32(scan_t *s)
{
    uint64_t value = scan_u64(s);

    if (value > UINT32_MAX)
        s->bad = true;
    return ((uint32_t)value);
}

int64_t scan_s64(scan_t *s)
{
    uint32_t shift;
    uint8_t last;
    uint64_t value = leb(s, &shift, &last);

    if (shift < 64 && (last & 0x40))
        value |= ~(uint64_t)0 << shift;
    return ((int64_t)value);
}

void scan_skip(scan_t *s, size_t n)
{
    if (s->bad || (size_t)(s->end - s->at) < n) {
        s->bad = true;
        return;
    }
    s->at += n;
}
