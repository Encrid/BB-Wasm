/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
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

static enum scan_step start(scan_job_t *job, scan_frame_t *f,
    uint32_t function)
{
    *f = (scan_frame_t){ .function = function };
    f->w = (scan_walk_t){ job, &job->m, &f->out, false, SCAN_NO_WAIT };
    job->match[function] = SCAN_BUSY;
    return (scan_begin(f, &job->m.bodies[function]));
}

static void close_frame(scan_job_t *job, scan_frame_t *f, bool walked)
{
    int32_t found = walked ? find_acc(job, &f->out) : -1;

    job->broken |= !walked;
    job->no_memory |= f->out.bad;
    job->touched[f->function] = f->w.touches;
    job->match[f->function] = found;
    free(f->out.bytes);
    f->out = (scan_out_t){ };
}

void scan_match_from(scan_job_t *job, uint32_t root)
{
    scan_frame_t *f = job->frames;
    enum scan_step step = start(job, f, root);
    uint32_t callee = 0;

    while (step == SCAN_WAITS || f != job->frames) {
        if (step == SCAN_WAITS) {
            callee = f->w.wait;
            f->w.wait = SCAN_NO_WAIT;
            f++;
            step = start(job, f, callee);
        } else {
            close_frame(job, f, step == SCAN_DONE);
            f--;
            step = scan_resume(f);
        }
    }
    close_frame(job, f, step == SCAN_DONE);
}
