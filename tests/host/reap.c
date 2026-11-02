/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "cases.h"

void test_state_no_threads(host_t *h)
{
    h->cfg.threads = (uint32_t)strtol(h->arg, nullptr, 10);
    host_open_only(h);
}
