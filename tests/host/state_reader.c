/*
** Copyright (C) 2026 BB-Wasm Owner
**
** This file is part of BB-Wasm.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/state_reader.c -- how this program states the reader of the
** mover.
*/

#include "cases.h"

static const char *const TEST_READER_GRANTS[] = {
    "game:Position",
    "game:Marked"
};

/*
** test_state_reader_limited
**
** States the reader of the mover, on the wasm backend, with its grant.
*/
void test_state_reader_limited(bridge_artifact_t *a)
{
    host_state_wasm(a, "view:reader@0.1.0", "reader");
    a->cfg.read = TEST_READER_GRANTS;
    a->cfg.read_count = 2;
}

