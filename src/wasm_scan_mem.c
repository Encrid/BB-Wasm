/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "wasm_scan.h"

static bool put_role(scan_walk_t *w, uint32_t memory)
{
    uint8_t role;

    if (memory >= w->m->memories)
        return (false);
    role = (uint8_t)w->m->roles[memory];
    w->touches |= role == SCAN_SHARED || role == SCAN_PRIVATE;
    scan_put(w->out, &role, 1);
    return (true);
}

bool scan_memarg(scan_walk_t *w, scan_t *s, enum scan_kind kind)
{
    uint32_t align = scan_u32(s);
    uint32_t memory = align & 0x40 ? scan_u32(s) : 0;
    const uint8_t *lane;

    scan_put_u64(w->out, align & ~(uint32_t)0x40);
    scan_put_u64(w->out, scan_u64(s));
    lane = s->at;
    if (kind == SCAN_MEMARG_LANE)
        scan_skip(s, 1);
    if (kind == SCAN_MEMARG_LANE && !s->bad)
        scan_put(w->out, lane, 1);
    return (!s->bad && put_role(w, memory));
}

bool scan_memory(scan_walk_t *w, scan_t *s,
    [[maybe_unused]] enum scan_kind kind)
{
    uint32_t memory = scan_u32(s);

    return (!s->bad && put_role(w, memory));
}

bool scan_copy(scan_walk_t *w, scan_t *s, [[maybe_unused]] enum scan_kind kind)
{
    uint32_t to = scan_u32(s);
    uint32_t from = scan_u32(s);

    return (!s->bad && put_role(w, to) && put_role(w, from));
}

bool scan_init(scan_walk_t *w, scan_t *s, [[maybe_unused]] enum scan_kind kind)
{
    uint32_t data = scan_u32(s);
    uint32_t memory = scan_u32(s);

    scan_put_u64(w->out, data);
    return (!s->bad && put_role(w, memory));
}
