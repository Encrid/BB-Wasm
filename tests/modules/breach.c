/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "game.h"
#include "wasm_idle.h"

constexpr uint32_t BREACH_ENTITIES = 4;

static uint32_t position;
static uint32_t moving;
#if defined(BREACH_TOGGLE) || defined(BREACH_OK)
static uint32_t lit;
#endif
#if defined(BREACH_OK)
static uint64_t ids[BREACH_ENTITIES];
#endif
#if defined(BREACH_SPARSE)
static uint32_t marked;
#endif
#if defined(BREACH_DIRECT)
static uint32_t dotted;
#endif

#if defined(BREACH_ROW) || defined(BREACH_WHO) || defined(BREACH_TOGGLE) \
    || defined(BREACH_OK)
static uint32_t a_table(void)
{
    return (bridge_get_span_table(bridge_get_span(moving, 0)));
}
#endif

#if defined(BREACH_OK)
static uint32_t get_last_found(void)
{
    uint64_t last = bridge_get_entity(a_table(), BREACH_ENTITIES - 1);
    uint32_t found = 0;

    (void)BRIDGE_GET(position_t, x, position, a_table(), BREACH_ENTITIES - 1);
    (void)bridge_is_on(lit, a_table(), BREACH_ENTITIES - 1);
    (void)acc_get_rows(BRIDGE_HEADER(tables) - 1);
    for (uint32_t i = 0; i < BREACH_ENTITIES; i++)
        found += ids[i] == last;
    return (found);
}
#endif

BRIDGE_EXPORT(breach) int breach(void)
{
#if defined(BREACH_SLOT)
    (void)acc_get_32(50, 0, 0);
#elif defined(BREACH_ROW)
    (void)BRIDGE_GET(position_t, x, position, a_table(), BREACH_ENTITIES);
#elif defined(BREACH_WHO)
    (void)bridge_get_entity(a_table(), BREACH_ENTITIES);
#elif defined(BREACH_TOGGLE)
    (void)bridge_is_on(lit, a_table(), BREACH_ENTITIES);
#elif defined(BREACH_ROWS)
    (void)acc_get_rows(BRIDGE_HEADER(tables));
#elif defined(BREACH_FIELD)
    acc_set_32(position, BRIDGE_AT(0, 0), 1u << 20, 0);
#elif defined(BREACH_READONLY)
    acc_set_32(position, BRIDGE_AT(0, 0), 0, 0);
#elif defined(BREACH_FORGE)
    acc_set_own_32(BRIDGE_HEADER(queries), 0);
#elif defined(BREACH_OWN_EDGE)
    acc_set_own_32(BRIDGE_HEADER(queries) - 3, 0);
#elif defined(BREACH_OWN_BELOW)
    acc_set_own_32(BRIDGE_HEADER(queries) - 4, 0);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "below", BREACH_ENTITIES, 0);
#elif defined(BREACH_SPARSE)
    (void)BRIDGE_GET(marked_t, by, marked, 1u << 20, 0);
#elif defined(BREACH_DIRECT)
    (void)BRIDGE_GET(marked_t, by, dotted, 1u << 20, 0);
#elif defined(BREACH_TO_SHARED)
    uint32_t word = 0;

    acc_copy_to_shared(0, &word, sizeof(word));
#elif defined(BREACH_FROM_SHARED)
    uint32_t word = 0;

    acc_copy_from_shared(&word, 0, sizeof(word));
    (void)word;
#elif defined(BREACH_OK)
    BRIDGE_LOG(BRIDGE_LOG_INFO, "ok", BREACH_ENTITIES, get_last_found());
#endif
    return (0);
}

static void populate(void)
{
    position_t p = { 0.0f, 0.0f };
#if defined(BREACH_TOGGLE) || defined(BREACH_OK)
    marked_t on = {};
#endif
    uint64_t id;
    uint32_t i = 0;

    while (i < BREACH_ENTITIES) {
        id = bridge_spawn();
        BRIDGE_ATTACH(id, position, p);
#if defined(BREACH_TOGGLE) || defined(BREACH_OK)
        BRIDGE_ATTACH(id, lit, on);
#endif
#if defined(BREACH_OK)
        ids[i] = id;
#endif
        i++;
    }
}

BRIDGE_EXPORT(init) int init(void)
{
#if defined(BREACH_FIELD)
    bridge_term_t term[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};
#else
    bridge_term_t term[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
#endif

    position = BRIDGE_COMPONENT("game:Position", position_t, BRIDGE_DELETE,
        BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE);
#if defined(BREACH_TOGGLE) || defined(BREACH_OK)
    lit = BRIDGE_COMPONENT("game:Lit", marked_t, BRIDGE_DELETE,
        BRIDGE_TOGGLE, BRIDGE_ON_DESTROY_REMOVE);
#endif
#if defined(BREACH_SPARSE)
    marked = BRIDGE_COMPONENT("game:Mark", marked_t, BRIDGE_DELETE,
        BRIDGE_SPARSE_SET, BRIDGE_ON_DESTROY_REMOVE);
#endif
#if defined(BREACH_DIRECT)
    dotted = BRIDGE_COMPONENT("game:Dot", marked_t, BRIDGE_DELETE,
        BRIDGE_DIRECT_INDEXED, BRIDGE_ON_DESTROY_REMOVE);
#endif
    term[0].slot = position;
    moving = BRIDGE_ADD_SYSTEM("breach", "update", term, 1, nullptr, 0,
        nullptr, 0);
    populate();
    return (0);
}
