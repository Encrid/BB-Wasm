/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "wasm_scan.h"

bool scan_none([[maybe_unused]] scan_walk_t *w, [[maybe_unused]] scan_t *s,
    [[maybe_unused]] enum scan_kind kind)
{
    return (true);
}

bool scan_raw(scan_walk_t *w, scan_t *s, enum scan_kind kind)
{
    size_t n = 1;
    const uint8_t *at = s->at;

    if (kind == SCAN_F32)
        n = 4;
    if (kind == SCAN_F64)
        n = 8;
    if (kind == SCAN_V128)
        n = 16;
    scan_skip(s, n);
    if (!s->bad)
        scan_put(w->out, at, n);
    return (!s->bad);
}

bool scan_number(scan_walk_t *w, scan_t *s, enum scan_kind kind)
{
    if (kind == SCAN_I32 || kind == SCAN_I64)
        scan_put_u64(w->out, (uint64_t)scan_s64(s));
    else
        scan_put_u64(w->out, scan_u32(s));
    return (!s->bad);
}

bool scan_labels(scan_walk_t *w, scan_t *s,
    [[maybe_unused]] enum scan_kind kind)
{
    uint32_t count = scan_u32(s);

    scan_put_u64(w->out, count);
    while (count && !s->bad) {
        scan_put_u64(w->out, scan_u32(s));
        count--;
    }
    scan_put_u64(w->out, scan_u32(s));
    return (!s->bad);
}

bool scan_types(scan_walk_t *w, scan_t *s, [[maybe_unused]] enum scan_kind kind)
{
    uint32_t count = scan_u32(s);
    const uint8_t *at = s->at;

    scan_put_u64(w->out, count);
    scan_skip(s, count);
    if (!s->bad)
        scan_put(w->out, at, count);
    return (!s->bad);
}
