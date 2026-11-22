/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "wasm_scan.h"

static const scan_handler_t SCAN_HANDLERS[SCAN_KIND_COUNT] = {
    [SCAN_NONE] = scan_none, [SCAN_BLOCK] = scan_block,
    [SCAN_LABEL] = scan_number, [SCAN_LABELS] = scan_labels,
    [SCAN_CALL] = scan_call, [SCAN_CALL_INDIRECT] = scan_two,
    [SCAN_INDEX] = scan_number, [SCAN_MEMARG] = scan_memarg,
    [SCAN_MEMORY] = scan_memory, [SCAN_I32] = scan_number,
    [SCAN_I64] = scan_number, [SCAN_F32] = scan_raw, [SCAN_F64] = scan_raw,
    [SCAN_TYPES] = scan_types, [SCAN_REF_TYPE] = scan_raw,
    [SCAN_PREFIX_FC] = scan_prefix, [SCAN_PREFIX_FD] = scan_prefix,
    [SCAN_INIT] = scan_init, [SCAN_COPY] = scan_copy, [SCAN_TWO] = scan_two,
    [SCAN_V128] = scan_raw, [SCAN_LANE] = scan_raw,
    [SCAN_MEMARG_LANE] = scan_memarg
};

bool scan_immediate(scan_walk_t *w, scan_t *s, enum scan_kind kind)
{
    if (kind == SCAN_BAD)
        return (false);
    return (SCAN_HANDLERS[kind](w, s, kind) && !s->bad);
}

static void put_run(scan_walk_t *w, uint64_t run, uint8_t type)
{
    if (!run)
        return;
    scan_put_u64(w->out, run);
    scan_put(w->out, &type, 1);
}

static bool read_locals(scan_walk_t *w, scan_t *s)
{
    uint32_t groups = scan_u32(s);
    uint64_t run = 0;
    uint64_t count;
    uint8_t last = 0;
    uint8_t type;

    while (groups && !s->bad) {
        count = scan_u32(s);
        type = scan_byte(s);
        if (type != last)
            put_run(w, run, last);
        run = type == last ? run + count : count;
        last = type;
        groups--;
    }
    put_run(w, run, last);
    return (!s->bad);
}

bool scan_begin(scan_frame_t *f, const scan_body_t *body)
{
    f->s = (scan_t){ body->at, body->at + body->len, false };
    f->depth = 1;
    return (read_locals(&f->w, &f->s));
}

enum scan_step scan_resume(scan_frame_t *f)
{
    scan_t back = f->s;
    size_t len = f->w.out->len;
    uint8_t op;

    while (f->depth) {
        back = f->s;
        len = f->w.out->len;
        op = scan_byte(&f->s);
        scan_put(f->w.out, &op, 1);
        if (!scan_immediate(&f->w, &f->s, scan_kind_of(op)))
            break;
        f->depth += op >= 0x02 && op <= 0x04;
        f->depth -= op == 0x0B;
    }
    if (!f->depth || f->w.wait == SCAN_NO_WAIT)
        return (!f->depth && f->s.at == f->s.end && !f->w.out->bad
            ? SCAN_DONE : SCAN_FAILED);
    f->s = back;
    f->w.out->len = len;
    return (SCAN_WAITS);
}
