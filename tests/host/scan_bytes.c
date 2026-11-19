/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "cases.h"

constexpr size_t TEST_HEX_BYTES = 512;

static uint8_t test_hex[TEST_HEX_BYTES];

static size_t test_unhex(const char *text)
{
    size_t n = 0;
    unsigned byte;

    while (n < TEST_HEX_BYTES && sscanf(text + 2 * n, "%2x", &byte) == 1) {
        test_hex[n] = (uint8_t)byte;
        n++;
    }
    return (n);
}

void test_load_hex(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_small(&a, identity, "ok");
    a.len = test_unhex(h->arg);
    a.bytes = test_hex;
    host_load(h, &a);
    host_close(h);
}
