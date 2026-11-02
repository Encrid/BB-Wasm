/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "wasm_idle.h"

#ifdef SHORT
    #define LENGTH 1020
#else
    #define LENGTH 1024
#endif

constexpr uint32_t VECTOR_COUNT = LENGTH;

static uint32_t values[VECTOR_COUNT];

static uint32_t get_sum(void)
{
    uint32_t sum = 0;

    for (uint32_t i = 0; i < VECTOR_COUNT; i++)
        sum += values[i];
    return (sum);
}

BRIDGE_EXPORT(add) int add(void)
{
    BRIDGE_LOG(BRIDGE_LOG_INFO, "sum", get_sum(), VECTOR_COUNT);
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    for (uint32_t i = 0; i < VECTOR_COUNT; i++)
        values[i] = i;
    BRIDGE_ADD_SYSTEM("add", "update", nullptr, 0, nullptr, 0, nullptr, 0);
    return (0);
}
