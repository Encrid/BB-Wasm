/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wasm_scan.h"

static int32_t find_acc(const scan_job_t *job, const scan_out_t *out)
{
    uint32_t a = 0;
    const scan_out_t *canon;

    while (a < job->acc.defined) {
        canon = &job->canons[a];
        if (canon->len == out->len && !memcmp(canon->bytes, out->bytes,
            out->len))
            return ((int32_t)a);
        a++;
    }
    return (-1);
}

static int32_t match_one(scan_job_t *job, uint32_t function)
{
    scan_out_t out = { };
    scan_walk_t w = { job, &job->m, &out, false };
    int32_t found = -1;

    if (scan_walk(&w, &job->m.bodies[function]))
        found = find_acc(job, &out);
    else
        job->broken = true;
    job->no_memory |= out.bad;
    job->touched[function] = w.touches;
    free(out.bytes);
    return (found);
}

int32_t scan_matched(scan_walk_t *w, uint32_t function)
{
    scan_job_t *job = w->job;

    if (!job->match)
        return ((int32_t)function);
    if (function >= job->m.defined)
        return (-1);
    if (job->match[function] == SCAN_BUSY)
        return (-1);
    if (job->match[function] != SCAN_UNKNOWN)
        return (job->match[function]);
    job->match[function] = SCAN_BUSY;
    job->match[function] = match_one(job, function);
    return (job->match[function]);
}

static void check_all(scan_job_t *job)
{
    scan_walk_t w = { job, &job->m, nullptr, false };
    uint32_t f = 0;
    int32_t found;

    while (f < job->m.defined && !job->m.refused) {
        found = scan_matched(&w, f);
        if (job->broken)
            scan_refuse(&job->m, "its function", job->m.imported + f,
                "holds code that the check cannot read");
        else if (job->touched[f] && found < 0)
            scan_refuse(&job->m, "its function", job->m.imported + f,
                "reaches a memory of Bridge outside the accessors");
        f++;
    }
}

const char *scan_refused(const uint8_t *bytes, size_t len, char *why,
    size_t size)
{
    scan_job_t job;
    bool loaded = scan_load(&job, bytes, len);

    if (loaded)
        check_all(&job);
    job.no_memory |= job.acc.no_memory || job.m.no_memory;
    if (job.no_memory)
        snprintf(why, size, "out of memory in the check of the module");
    else if (!loaded)
        snprintf(why, size, "the module is refused: the check cannot read it");
    else if (job.m.refused)
        snprintf(why, size, "the module is refused: %s", job.m.why);
    loaded = loaded && !job.m.refused;
    scan_free_job(&job);
    return (loaded ? nullptr : why);
}
