/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "wasm_scan.h"

void scan_functions(scan_module_t *m, scan_t *s)
{
    uint32_t count = scan_u32(s);
    uint32_t i = 0;

    if (s->bad || count > SCAN_FUNCTIONS || m->bodies) {
        s->bad = true;
        return;
    }
    m->bodies = calloc(count ? count : 1, sizeof(*m->bodies));
    if (!m->bodies) {
        m->no_memory = true;
        s->bad = true;
        return;
    }
    m->defined = count;
    while (i < count && !s->bad) {
        (void)scan_u32(s);
        i++;
    }
}

void scan_codes(scan_module_t *m, scan_t *s)
{
    uint32_t count = scan_u32(s);
    uint32_t i = 0;

    if (s->bad || count != m->defined) {
        s->bad = true;
        return;
    }
    while (i < count && !s->bad) {
        m->bodies[i].len = scan_u32(s);
        m->bodies[i].at = s->at;
        scan_skip(s, m->bodies[i].len);
        i++;
    }
}
