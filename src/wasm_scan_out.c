/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wasm_scan.h"

void scan_put(scan_out_t *o, const void *bytes, size_t n)
{
    size_t cap = o->cap ? o->cap : 256;
    uint8_t *grown;

    if (o->bad)
        return;
    while (cap < o->len + n)
        cap *= 2;
    if (cap != o->cap) {
        grown = realloc(o->bytes, cap);
        if (!grown) {
            o->bad = true;
            return;
        }
        o->bytes = grown;
        o->cap = cap;
    }
    memcpy(o->bytes + o->len, bytes, n);
    o->len += n;
}

void scan_put_u64(scan_out_t *o, uint64_t value)
{
    uint8_t bytes[8];
    uint32_t i = 0;

    while (i < 8) {
        bytes[i] = (uint8_t)(value >> (8 * i));
        i++;
    }
    scan_put(o, bytes, sizeof(bytes));
}

void scan_refuse(scan_module_t *m, const char *head, uint32_t index,
    const char *tail)
{
    if (m->refused)
        return;
    m->refused = 1;
    snprintf(m->why, sizeof(m->why), "%s %u %s", head, index, tail);
}

void scan_free(scan_module_t *m)
{
    free(m->bodies);
    m->bodies = nullptr;
}
