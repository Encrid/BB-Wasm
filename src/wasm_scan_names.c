/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "wasm_scan.h"

uint8_t scan_byte(scan_t *s)
{
    if (s->bad || s->at >= s->end) {
        s->bad = true;
        return (0);
    }
    return (*s->at++);
}

void scan_name(scan_t *s, scan_name_t *name)
{
    name->len = scan_u32(s);
    name->at = s->at;
    scan_skip(s, name->len);
}

bool scan_name_is(const scan_name_t *name, const char *text)
{
    return (name->len == strlen(text) && !memcmp(name->at, text, name->len));
}

void scan_limits(scan_t *s)
{
    uint8_t flags = scan_byte(s);

    (void)scan_u64(s);
    if (flags & 1)
        (void)scan_u64(s);
}
