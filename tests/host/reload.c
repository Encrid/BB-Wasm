/*
** Copyright (C) 2026 BB-Wasm Owner
**
** This file is part of BB-Wasm.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/reload.c -- the ticker reloaded: into a build that
** counts by ten, one with another component layout, one whose reload
** fails, one that no longer declares what it did; with
** more ids than the old instance held ready; and where the deployment says
** it may not be.
**
** lifetime.md, *When a component layout changed*
*/

#include <string.h>

#include "cases.h"

/*
** test_reload_nothing
**
** A reload of a module that is not loaded.
*/
void test_reload_nothing(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_ticker(&a, "game:ticker@0.2.0", "ticker-ten");
    host_reload(h, &a);
    host_close(h);
}

/*
** test_revive
**
** A stopped module, reloaded into one that works.
*/
void test_revive(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_ticker(&a, "game:ticker@0.1.0", "ticker-trip");
    host_load(h, &a);
    host_run_frames(h, 4);
    test_say_modules(h);
    test_state_ticker(&a, "game:ticker@0.2.0", "ticker-ten");
    host_reload(h, &a);
    host_run_frames(h, 2);
    test_say_modules(h);
    host_close(h);
}
