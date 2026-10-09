/*
** Copyright (C) 2026 BB-Wasm Owner
**
** This file is part of BB-Wasm.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/reap.c -- a module of another namespace that
** destroys what it finds: with a read grant it has no stake in the entity,
** which stays; with a write grant it has, and the entity goes. And two
** cases of threads: an entry reached from one nobody called on, and a
** deployment that states none.
**
** grants.md, *What a module may destroy*
*/

#include "cases.h"

/*
** test_state_no_threads
**
** A deployment that states no thread.
*/
void test_state_no_threads(host_t *h)
{
    h->cfg.threads = 0;
    host_open_only(h);
}
