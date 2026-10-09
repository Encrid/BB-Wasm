/*
** Copyright (C) 2026 BB-Wasm Owner
**
** This file is part of BB-Wasm.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/hear_counts.c -- the sink of this program: what it keeps of
** the records the cases count, and what it prints.
*/

#include <stdio.h>
#include <string.h>

#include "cases.h"

static bool is_text(const bridge_record_t *r, const char *text)
{
    return (r->len == 3 && !memcmp(r->text, text, 3));
}

/*
** keep_ids
**
** Keeps the two ids a spawner said it was given, in the run in progress.
*/
static void keep_ids(test_counts_t *c, const bridge_record_t *r)
{
    uint32_t *n = &c->minted_count[c->minting];

    if (*n + 2 > 256)
        return;
    c->minted[c->minting][*n] = r->arg[0];
    c->minted[c->minting][*n + 1] = r->arg[1];
    *n += 2;
}

/*
** keep_counted
**
** Keeps a record the cases count. Answers false if it is one to print.
*/
static bool keep_counted(const test_listener_t *l, const bridge_record_t *r)
{
    test_counts_t *c = l->counts;

    if (l->which >= 0 && is_text(r, "sum"))
        c->last_sum[l->which] = r->arg[0];
    if (l->which >= 0)
        return (true);
    if (c->minting >= 0 && is_text(r, "ids")) {
        keep_ids(c, r);
        return (true);
    }
    return (false);
}

/*
** test_hear_counts
**
** The sink of this program: keeps what the cases count, and prints every
** other record with its barrier.
*/
void test_hear_counts(void *ud, const bridge_record_t *r)
{
    const char *identity = r->identity ? r->identity : "-";
    const char *system = r->system ? r->system : "-";

    if (keep_counted(ud, r))
        return;
    printf("record[%d] b%u %s/%s: %.*s %u %u\n", r->level, r->when >> 16,
        identity, system, (int)r->len, r->text, r->arg[0], r->arg[1]);
}

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
