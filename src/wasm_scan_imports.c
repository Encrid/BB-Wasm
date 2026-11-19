/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "wasm_scan.h"

static enum scan_role role_of(const scan_name_t *module,
    const scan_name_t *field)
{
    if (scan_name_is(module, "bridge") && scan_name_is(field, "shared"))
        return (SCAN_SHARED);
    if (scan_name_is(module, "bridge") && scan_name_is(field, "private"))
        return (SCAN_PRIVATE);
    if (scan_name_is(module, "main") && scan_name_is(field, "memory"))
        return (SCAN_OWN);
    return (SCAN_OTHER);
}

void scan_add_memory(scan_module_t *m, scan_t *s, enum scan_role role)
{
    if (m->memories >= SCAN_MEMORIES) {
        s->bad = true;
        return;
    }
    m->roles[m->memories] = role;
    m->memories++;
}

static void import_kind(scan_module_t *m, scan_t *s, enum scan_role role)
{
    switch (scan_byte(s)) {
        case 0:
            (void)scan_u32(s);
            m->imported++;
            break;
        case 1:
            (void)scan_byte(s);
            scan_limits(s);
            break;
        case 2:
            scan_limits(s);
            scan_add_memory(m, s, role);
            break;
        case 3:
            scan_skip(s, 2);
            break;
        default:
            s->bad = true;
    }
}

void scan_imports(scan_module_t *m, scan_t *s)
{
    uint32_t count = scan_u32(s);
    scan_name_t module;
    scan_name_t field;

    while (count && !s->bad) {
        scan_name(s, &module);
        scan_name(s, &field);
        import_kind(m, s, role_of(&module, &field));
        count--;
    }
}

void scan_memories(scan_module_t *m, scan_t *s)
{
    uint32_t count = scan_u32(s);

    while (count && !s->bad) {
        scan_limits(s);
        scan_add_memory(m, s, SCAN_OWN);
        count--;
    }
}
