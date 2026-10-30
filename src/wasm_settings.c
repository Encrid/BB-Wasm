/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "wasm_inside.h"

static const char *const WASM_KEYS[WASM_SETTINGS] = {
    "reservation", "guard", "max_wasm_stack", "stack_margin", "watchdog_stack",
    "opt_level", "parallel_compilation"
};

static const enum backend_setting_kind WASM_KINDS[WASM_SETTINGS] = {
    BACKEND_INT, BACKEND_INT, BACKEND_INT, BACKEND_INT, BACKEND_INT,
    BACKEND_STRING, BACKEND_BOOL
};

static const char *const WASM_KIND_NAMES[] = {
    [BACKEND_BOOL] = "a flag",
    [BACKEND_INT] = "an integer",
    [BACKEND_STRING] = "a text"
};

static const char *const WASM_LEVELS[] = { "none", "speed", "speed_and_size" };

static int find_key(const char *key, backend_err_t *err)
{
    int k = 0;

    while (k < WASM_SETTINGS && strcmp(WASM_KEYS[k], key))
        k++;
    if (k == WASM_SETTINGS)
        snprintf(err->text, sizeof(err->text), "it knows no setting '%.100s'",
            key);
    return (k);
}

static int place(const backend_setting_t **given,
    const backend_setting_t *settings, uint32_t count, backend_err_t *err)
{
    const char *key;
    uint32_t n = 0;
    int k;

    while (n < count) {
        key = settings[n].key ? settings[n].key : "";
        k = find_key(key, err);
        if (k == WASM_SETTINGS)
            return (1);
        if (settings[n].kind != WASM_KINDS[k]) {
            snprintf(err->text, sizeof(err->text), "%s must be %s", key,
                WASM_KIND_NAMES[WASM_KINDS[k]]);
            return (1);
        }
        given[k] = &settings[n];
        n++;
    }
    return (0);
}

static int read_size(const backend_setting_t *const *given, int k,
    uint64_t *out, backend_err_t *err)
{
    int64_t v = given[k]->v.i;
    int64_t least = k == WASM_KEY_STACK ? 1 : 0;

    if (v >= least) {
        *out = (uint64_t)v;
        return (0);
    }
    snprintf(err->text, sizeof(err->text), "%s is %lld bytes, and it must be "
        "%lld or more", WASM_KEYS[k], (long long)v, (long long)least);
    return (1);
}

static int read_level(const backend_setting_t *s, wasmtime_opt_level_t *out,
    backend_err_t *err)
{
    uint8_t k = 0;

    while (k < 3 && strcmp(WASM_LEVELS[k], s->v.s))
        k++;
    if (k < 3) {
        *out = k;
        return (0);
    }
    snprintf(err->text, sizeof(err->text), "opt_level is '%.100s', and it "
        "can be none, speed or speed_and_size", s->v.s);
    return (1);
}

int wasm_read_settings(wasm_settings_t *s, const backend_setting_t *settings,
    uint32_t count, backend_err_t *err)
{
    const backend_setting_t *given[WASM_SETTINGS] = {};
    int k = 0;

    if (place(given, settings, count, err))
        return (1);
    while (k < WASM_SETTINGS && given[k])
        k++;
    if (k < WASM_SETTINGS) {
        snprintf(err->text, sizeof(err->text), "%s is unset", WASM_KEYS[k]);
        return (1);
    }
    s->parallel_compilation = given[WASM_KEY_COMPILE]->v.b;
    return (read_size(given, WASM_KEY_RESERVATION, &s->reservation, err)
        || read_size(given, WASM_KEY_GUARD, &s->guard, err)
        || read_size(given, WASM_KEY_STACK, &s->wasm_stack, err)
        || read_size(given, WASM_KEY_MARGIN, &s->stack_margin, err)
        || read_size(given, WASM_KEY_WATCHDOG, &s->watchdog_stack, err)
        || read_level(given[WASM_KEY_LEVEL], &s->opt_level, err));
}
