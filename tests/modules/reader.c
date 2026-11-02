/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "game.h"
#include "wasm_idle.h"

#ifdef READER_POSITION
    #define READER_TERMS 1
#else
    #define READER_TERMS 2
#endif
#ifdef READER_WARN
    #define READER_LEVEL BRIDGE_LOG_WARN
#else
    #define READER_LEVEL BRIDGE_LOG_INFO
#endif
#ifdef READER_TOTAL
    #define READER_TEXT "total"
#else
    #define READER_TEXT "sum"
#endif
#ifdef READER_ADD
    #define READER_SYSTEM "add"
#else
    #define READER_SYSTEM "sum"
#endif

static uint32_t position;
static uint32_t marked;
static uint32_t summing;

static uint32_t add_span(uint32_t span, float *total)
{
    uint32_t count = bridge_get_rows(span);
    uint32_t table = bridge_get_span_table(span);
    uint32_t r = 0;

    while (r < count) {
        *total += BRIDGE_GET(position_t, x, position, table, r);
        r++;
    }
    return (count);
}

#ifdef READER_ADD
BRIDGE_EXPORT(add) int add(void)
#else
BRIDGE_EXPORT(sum) int sum(void)
#endif
{
    uint32_t spans = bridge_get_span_count(summing);
    uint32_t rows = 0;
    uint32_t s = 0;
    float total = 0.0f;

    while (s < spans) {
        rows += add_span(bridge_get_span(summing, s), &total);
        s++;
    }
    BRIDGE_LOG(READER_LEVEL, "saw", spans, rows);
    BRIDGE_LOG(READER_LEVEL, READER_TEXT, (uint32_t)total, 0);
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[2] = {
        { 0, BRIDGE_READ, BRIDGE_AND, 0 },
        { 0, BRIDGE_READ, BRIDGE_AND, 0 },
    };

    position = BRIDGE_COMPONENT("game:Position", position_t, BRIDGE_STRIP,
        BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE);
    marked = BRIDGE_COMPONENT("game:Marked", marked_t, BRIDGE_STRIP,
        BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE);
    terms[0].slot = position;
    terms[1].slot = marked;
    summing = BRIDGE_ADD_SYSTEM(READER_SYSTEM, "present", terms, READER_TERMS,
        nullptr, 0, nullptr, 0);
    return (0);
}
