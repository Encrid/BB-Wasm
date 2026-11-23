/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "wasm_scan.h"

constexpr uint64_t SCAN_IMPORTED = (uint64_t)1 << 40;
constexpr uint64_t SCAN_UNMATCHED = (uint64_t)1 << 41;

bool scan_block(scan_walk_t *w, scan_t *s, [[maybe_unused]] enum scan_kind kind)
{
    scan_put_u64(w->out, (uint64_t)scan_s64(s));
    return (!s->bad);
}

bool scan_call(scan_walk_t *w, scan_t *s, [[maybe_unused]] enum scan_kind kind)
{
    uint32_t function = scan_u32(s);
    int32_t matched;

    if (s->bad)
        return (false);
    if (function < w->m->imported) {
        scan_put_u64(w->out, SCAN_IMPORTED | function);
        return (true);
    }
    matched = scan_matched(w, function - w->m->imported);
    if (matched == SCAN_UNKNOWN) {
        w->wait = function - w->m->imported;
        return (false);
    }
    scan_put_u64(w->out, matched < 0 ? SCAN_UNMATCHED : (uint64_t)matched);
    return (true);
}

int32_t scan_matched(const scan_walk_t *w, uint32_t function)
{
    const scan_job_t *job = w->job;

    if (!job->match)
        return ((int32_t)function);
    if (function >= job->m.defined || job->match[function] == SCAN_BUSY)
        return (-1);
    return (job->match[function]);
}

bool scan_two(scan_walk_t *w, scan_t *s, [[maybe_unused]] enum scan_kind kind)
{
    scan_put_u64(w->out, scan_u32(s));
    scan_put_u64(w->out, scan_u32(s));
    return (!s->bad);
}

bool scan_prefix(scan_walk_t *w, scan_t *s, enum scan_kind kind)
{
    uint32_t op = scan_u32(s);

    scan_put_u64(w->out, op);
    if (s->bad)
        return (false);
    if (kind == SCAN_PREFIX_FC)
        return (scan_immediate(w, s, scan_kind_of_fc(op)));
    return (scan_immediate(w, s, scan_kind_of_fd(op)));
}
