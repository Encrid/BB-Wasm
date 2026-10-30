/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "cases.h"

void test_limit_elements(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_small(&a, identity, "tablegrow");
    a.cfg.element_limit = atoll(h->arg);
    host_load(h, &a);
    host_close(h);
}

void test_load_lent(host_t *h)
{
    if (host_open(h))
        return;
    test_load_mover(h);
    host_close(h);
}
