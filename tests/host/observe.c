/*
** Copyright (C) 2026 BB-Wasm Owner
**
** This file is part of BB-Wasm.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/observe.c -- observers: two of one change, in two modules,
** one of them writing what the other reads. The barrier is a phase like any
** other to the conflict graph.
*/

#include "cases.h"

/*
** test_take_reserved_name
**
** An observer that takes a name that is a lifecycle export's.
*/
void test_take_reserved_name(host_t *h)
{
    if (host_open(h))
        return;
    test_load_wasm(h, "game:badobs@0.1.0", "badobs");
    host_close(h);
}
