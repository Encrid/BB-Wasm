/*
** Copyright (C) 2026 BB-Wasm Owner
**
** This file is part of BB-Wasm.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/refused_bytes.c -- what a backend cannot keep or cannot
** find, refused at the load.
**
** capabilities.md, *Nothing is downgraded*
*/

#include "cases.h"

/*
** test_state_no_bytes
**
** A wasm module, and nothing to compile.
*/
void test_state_no_bytes(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_reader_limited(&a);
    a.bytes = nullptr;
    a.len = 0;
    host_load(h, &a);
    host_close(h);
}

