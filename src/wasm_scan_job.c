/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "wasm_scan.h"

static bool make_canons(scan_job_t *job)
{
    scan_job_t same = { .acc = job->acc, .m = job->acc };
    scan_walk_t w = { &same, &job->acc, nullptr, false };
    uint32_t a = 0;

    job->canons = calloc(job->acc.defined + 1, sizeof(*job->canons));
    job->no_memory = !job->canons;
    if (!job->canons)
        return (false);
    while (a < job->acc.defined) {
        w.out = &job->canons[a];
        if (!scan_walk(&w, &job->acc.bodies[a])) {
            job->no_memory = job->canons[a].bad;
            return (false);
        }
        a++;
    }
    return (true);
}

static bool make_memos(scan_job_t *job)
{
    uint32_t count = job->m.defined + 1;
    uint32_t i = 0;

    job->match = malloc(count * sizeof(*job->match));
    job->touched = calloc(count, sizeof(*job->touched));
    job->no_memory = !job->match || !job->touched;
    if (job->no_memory)
        return (false);
    while (i < count) {
        job->match[i] = SCAN_UNKNOWN;
        i++;
    }
    return (true);
}

bool scan_load(scan_job_t *job, const uint8_t *bytes, size_t len)
{
    size_t acc_len = 0;
    const uint8_t *acc = scan_acc(&acc_len);

    *job = (scan_job_t){ };
    if (!scan_read(&job->acc, acc, acc_len) || !make_canons(job))
        return (false);
    if (!scan_read(&job->m, bytes, len))
        return (false);
    return (make_memos(job));
}

void scan_free_job(scan_job_t *job)
{
    uint32_t a = 0;

    while (job->canons && a < job->acc.defined) {
        free(job->canons[a].bytes);
        a++;
    }
    free(job->canons);
    free(job->match);
    free(job->touched);
    scan_free(&job->acc);
    scan_free(&job->m);
    *job = (scan_job_t){ };
}
