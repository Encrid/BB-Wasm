/*
** Copyright (C) 2026 BB-Wasm Owner
**
** This file is part of BB-Wasm.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/changes_stated.c -- what a module stated it changes: the
** bytes of its queue, and the components a system attaches.
**
** budgets.md, *Nothing is allocated in a phase*
*/

#include "cases.h"

/*
** test_hop_threads
**
** Wasm instances on four threads: a store is called from whichever thread
** took its module's job, never from two at once. Fifty frames, and the last
** sum is exact.
*/
void test_hop_threads(host_t *h)
{
    h->cfg.threads = 4;
    if (host_open(h))
        return;
    test_load_wasm(h, "game:mover@0.1.0", "mover");
    test_load_game_reader(h);
    host_run_frames_unsaid(h->b, 50);
    host_close(h);
}
