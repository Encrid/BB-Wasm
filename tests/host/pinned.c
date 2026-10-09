/*
** Copyright (C) 2026 BB-Wasm Owner
**
** This file is part of BB-Wasm.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/pinned.c -- a pinned module: its code runs on the
** thread that loaded it, or not at all.
**
** threads.md, *Threads*
*/

#include <stdio.h>

#include "cases.h"

/*
** test_get_counts
**
** Answers what the host's sink counts.
*/
test_counts_t *test_get_counts(const host_t *h)
{
    const test_listener_t *l = h->ud;

    return (l->counts);
}

