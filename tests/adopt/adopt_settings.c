/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "adopt.h"

static const backend_setting_t ADOPT_SETTINGS[ADOPT_SETTING_COUNT] = {
    { .key = "reservation", .kind = BACKEND_INT, .v = { .i = 1ll << 32 } },
    { .key = "guard", .kind = BACKEND_INT, .v = { .i = 64 << 20 } },
    { .key = "max_wasm_stack", .kind = BACKEND_INT },
    { .key = "stack_margin", .kind = BACKEND_INT, .v = { .i = ADOPT_MARGIN } },
    { .key = "watchdog_stack", .kind = BACKEND_INT, .v = { .i = 64 << 10 } },
    { .key = "opt_level", .kind = BACKEND_STRING, .v = { .s = "speed" } },
    { .key = "parallel_compilation", .kind = BACKEND_BOOL, .v = { .b = false } }
};

backend_t *adopt_start_backend(const adopt_t *t, int64_t stack,
    backend_err_t *err)
{
    backend_setting_t settings[ADOPT_SETTING_COUNT];

    memcpy(settings, ADOPT_SETTINGS, sizeof(settings));
    settings[2].v.i = stack;
    return (t->api.backend_create(settings, ADOPT_SETTING_COUNT, err));
}
