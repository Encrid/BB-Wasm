/*
** Copyright (C) 2026 BB-Wasm Owner
**
** This file is part of BB-Wasm.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/fixtures.c -- the fixtures of the host program: what a case's
** host is before the case changes it.
*/

#include "cases.h"

/*
** test_set_small
**
** The one phase "update", and the sink that prints a record's level.
*/
void test_set_small(host_t *h)
{
    h->cfg.phase_count = 1;
    h->sink = test_print;
}

/*
** test_set_wasm
**
** Both phases, and the sink that prints a record.
*/
void test_set_wasm(host_t *h)
{
    h->sink = host_print_record;
}

/*
** test_set_counting
**
** Both phases, and the sink that counts.
*/
void test_set_counting(host_t *h)
{
    test_kept_t *kept = h->kept;

    h->sink = test_hear_counts;
    h->ud = &kept->listener;
}

/*
** test_set_noting
**
** Both phases, and the sink that notes.
*/
void test_set_noting(host_t *h)
{
    test_kept_t *kept = h->kept;

    h->sink = test_hear_noted;
    h->ud = &kept->noted;
}
