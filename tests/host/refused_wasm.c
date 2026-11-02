/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cases.h"

void test_create_no_sink(host_t *h)
{
    bridge_sink_fn_t sink = strcmp(h->arg, "sink") ? nullptr : h->sink;
    bridge_world_t *w = bridge_create(&h->cfg, sink, h->ud);

    printf("%s\n", w ? "created" : "refused: no sink");
    if (w)
        (void)bridge_destroy(w);
    printf("alive\n");
}

void test_state_other_abi(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_small(&a, identity, "ok");
    a.cfg.abi = BRIDGE_ABI + (uint32_t)strtol(h->arg, nullptr, 10);
    host_load(h, &a);
    host_close(h);
}

void test_name_no_backend(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_small(&a, identity, "ok");
    a.backend = h->arg;
    host_load(h, &a);
    host_close(h);
}

void test_open_no_library(host_t *h)
{
    if (*h->arg)
        h->backends[0].library = h->arg;
    if (host_open(h))
        return;
    test_load_small(h, "ok");
    host_close(h);
}

void test_state_other_host_abi(host_t *h)
{
    h->cfg.abi = BRIDGE_HOST_ABI + (uint32_t)strtol(h->arg, nullptr, 10);
    if (host_open(h))
        return;
    test_load_small(h, "ok");
    host_close(h);
}
