/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "cases.h"

void test_run_named(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    snprintf(identity, sizeof(identity), "test:%s@0.1.0", h->arg);
    host_state_wasm(&a, identity, h->arg);
    host_load(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}
