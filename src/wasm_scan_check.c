/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "wasm_scan.h"

static void check_all(scan_job_t *job)
{
    uint32_t f = 0;

    while (f < job->m.defined && !job->m.refused) {
        if (job->match[f] == SCAN_UNKNOWN)
            scan_match_from(job, f);
        if (job->broken)
            scan_refuse(&job->m, "its function", job->m.imported + f,
                "holds code that the check cannot read");
        else if (job->touched[f] && job->match[f] < 0)
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
