/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>

#include "adopt.h"

static const enum backend_type ADOPT_ROOM_RESULT[1] = { BACKEND_I32 };

static int use_stack([[maybe_unused]] void *ud,
    [[maybe_unused]] const uint64_t *args, uint64_t *results)
{
    volatile uint8_t room[ADOPT_ROOM];

    room[0] = 1;
    room[ADOPT_ROOM - 1] = 2;
    results[0] = room[0] + room[ADOPT_ROOM - 1];
    return (0);
}

static int start_side(adopt_margin_t *m, adopt_side_t *s, int64_t stack)
{
    backend_sig_t sig = { .results = ADOPT_ROOM_RESULT, .result_count = 1 };
    backend_entry_t entry = { "bridge", "use_stack", use_stack, nullptr, &sig };
    backend_source_t source = { .bytes = m->bytes, .len = m->len };

    s->be = adopt_start_backend(m->t, stack, &m->err);
    if (s->be)
        s->module = m->t->api.module_compile(s->be, &source, &m->err);
    if (s->module)
        s->instance = m->t->api.instance_create(s->module, nullptr, 0,
            &m->err);
    if (s->instance && !m->t->api.define_func(s->instance, &entry, &m->err))
        s->f = m->t->api.instance_get_func(s->instance, "init", &m->err);
    return (!s->f);
}

static void run_both(void *arg)
{
    adopt_margin_t *m = arg;
    unsigned long long left = 0;
    int32_t status = 0;

    if (start_side(m, &m->side[0], 1ll << 30)
        || !m->t->api.call(m->side[0].f, &status, &m->err))
        return;
    sscanf(m->err.text, "did not run: the thread has %llu", &left);
    m->left = left;
    if (start_side(m, &m->side[1], (int64_t)left - ADOPT_MARGIN + m->below))
        return;
    m->called = true;
    (void)m->t->api.call(m->side[1].f, &status, &m->err);
}

static void stop_side(const adopt_t *t, const adopt_side_t *s)
{
    if (s->instance)
        t->api.instance_destroy(s->instance);
    if (s->module)
        t->api.module_destroy(s->module);
    if (s->be)
        t->api.backend_destroy(s->be);
}

int adopt_check_margin(adopt_t *t, bool below)
{
    adopt_margin_t m = { .t = t, .below = below };
    os_thread_t *thread;

    m.bytes = adopt_read_file(MODULE_DIR "/margin.wasm", &m.len);
    thread = os_thread_start(run_both, &m, ADOPT_STACK);
    if (thread)
        os_thread_join(thread);
    stop_side(t, &m.side[0]);
    stop_side(t, &m.side[1]);
    free(m.bytes);
    if (!m.called)
        return (adopt_fail("the margin: %s", m.err.text));
    if (below)
        printf("below the threshold: %s\n", m.err.text);
    else
        printf("at the threshold of %llu bytes, the offered function ran at "
            "the deepest wasm frame: %s\n", m.left, m.err.text);
    return (0);
}
