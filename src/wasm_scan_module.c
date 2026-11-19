/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "wasm_scan.h"

static const uint8_t SCAN_HEADER[8] = { 0, 'a', 's', 'm', 1, 0, 0, 0 };

static const scan_section_t SCAN_SECTIONS[12] = {
    [2] = scan_imports, [3] = scan_functions, [5] = scan_memories,
    [7] = scan_exports, [10] = scan_codes, [11] = scan_datas
};

static void read_section(scan_module_t *m, scan_t *s, uint8_t id)
{
    if (id < 12 && SCAN_SECTIONS[id])
        SCAN_SECTIONS[id](m, s);
    else
        s->at = s->end;
}

bool scan_read(scan_module_t *m, const uint8_t *bytes, size_t len)
{
    scan_t s = { bytes, bytes + len, false };
    scan_t part;
    uint8_t id;
    uint32_t size;

    *m = (scan_module_t){ };
    if (len < sizeof(SCAN_HEADER) || memcmp(bytes, SCAN_HEADER, 8))
        return (false);
    s.at += sizeof(SCAN_HEADER);
    while (!s.bad && s.at < s.end) {
        id = scan_byte(&s);
        size = scan_u32(&s);
        part = (scan_t){ s.at, s.at + size, false };
        scan_skip(&s, size);
        if (!s.bad)
            read_section(m, &part, id);
        s.bad |= part.bad;
    }
    return (!s.bad);
}
