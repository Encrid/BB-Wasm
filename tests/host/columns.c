/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "cases.h"

void test_use_up_columns(host_t *h)
{
    h->cfg.budget.columns = (uint64_t)atoi(*h->arg ? h->arg : "0");
    if (host_open(h))
        return;
    test_load_mover(h);
    host_run_frames(h, 1);
    host_close(h);
}
