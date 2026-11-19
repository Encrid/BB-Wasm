/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "wasm_scan.h"

bool scan_is_bridge(const scan_module_t *m, uint32_t memory)
{
    return (memory < m->memories && (m->roles[memory] == SCAN_SHARED
        || m->roles[memory] == SCAN_PRIVATE));
}

void scan_exports(scan_module_t *m, scan_t *s)
{
    uint32_t count = scan_u32(s);
    scan_name_t name;
    uint8_t kind;
    uint32_t index;

    while (count && !s->bad) {
        scan_name(s, &name);
        kind = scan_byte(s);
        index = scan_u32(s);
        if (kind == 2 && scan_is_bridge(m, index))
            scan_refuse(m, "it exports the memory", index, "of Bridge");
        count--;
    }
}

static void skip_offset(scan_t *s)
{
    uint8_t op = scan_byte(s);

    if (op == 0x41 || op == 0x42)
        (void)scan_s64(s);
    else if (op == 0x23)
        (void)scan_u32(s);
    else
        s->bad = true;
    if (scan_byte(s) != 0x0B)
        s->bad = true;
}

static void data_entry(scan_module_t *m, scan_t *s, uint32_t index)
{
    uint32_t mode = scan_u32(s);
    uint32_t memory = 0;

    if (mode == 2)
        memory = scan_u32(s);
    if (mode == 0 || mode == 2)
        skip_offset(s);
    if (mode > 2)
        s->bad = true;
    if (mode != 1 && scan_is_bridge(m, memory))
        scan_refuse(m, "its data segment", index, "writes a memory of Bridge");
    scan_skip(s, scan_u32(s));
}

void scan_datas(scan_module_t *m, scan_t *s)
{
    uint32_t count = scan_u32(s);
    uint32_t index = 0;

    while (index < count && !s->bad) {
        data_entry(m, s, index);
        index++;
    }
}
