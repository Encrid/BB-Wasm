/*
** Copyright (C) 2026 BB-Wasm Owner
**
** This file is part of BB-Wasm.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/native_run.c -- how this program states a module, and the
** cases in which a native module runs.
*/

#include "cases.h"

static const char *const TEST_READER_GRANTS[] = {
    "game:Position",
    "game:Marked"
};

/*
** test_state_native
**
** States a native module that mints up to 1,024 ids between two barriers.
*/
void test_state_native(bridge_artifact_t *a, const char *identity,
    const char *path)
{
    host_state_native(a, identity, path);
    a->cfg.spawns = 1024;
}

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

