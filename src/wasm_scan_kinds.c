/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "wasm_scan.h"

static const uint8_t SCAN_KINDS[256] = {
    [0x00] = SCAN_NONE, [0x01] = SCAN_NONE, [0x02] = SCAN_BLOCK,
    [0x03] = SCAN_BLOCK, [0x04] = SCAN_BLOCK, [0x05] = SCAN_NONE,
    [0x0B] = SCAN_NONE, [0x0C] = SCAN_LABEL, [0x0D] = SCAN_LABEL,
    [0x0E] = SCAN_LABELS, [0x0F] = SCAN_NONE, [0x10] = SCAN_CALL,
    [0x11] = SCAN_CALL_INDIRECT, [0x1A] = SCAN_NONE, [0x1B] = SCAN_NONE,
    [0x1C] = SCAN_TYPES, [0x20] = SCAN_INDEX, [0x21] = SCAN_INDEX,
    [0x22] = SCAN_INDEX, [0x23] = SCAN_INDEX, [0x24] = SCAN_INDEX,
    [0x25] = SCAN_INDEX, [0x26] = SCAN_INDEX, [0x3F] = SCAN_MEMORY,
    [0x40] = SCAN_MEMORY, [0x41] = SCAN_I32, [0x42] = SCAN_I64,
    [0x43] = SCAN_F32, [0x44] = SCAN_F64, [0xD0] = SCAN_REF_TYPE,
    [0xD1] = SCAN_NONE, [0xD2] = SCAN_INDEX, [0xFC] = SCAN_PREFIX_FC,
    [0xFD] = SCAN_PREFIX_FD
};

static const uint8_t SCAN_KINDS_FC[18] = {
    SCAN_NONE, SCAN_NONE, SCAN_NONE, SCAN_NONE, SCAN_NONE, SCAN_NONE,
    SCAN_NONE, SCAN_NONE, SCAN_INIT, SCAN_INDEX, SCAN_COPY, SCAN_MEMORY,
    SCAN_TWO, SCAN_INDEX, SCAN_TWO, SCAN_INDEX, SCAN_INDEX, SCAN_INDEX
};

enum scan_kind scan_kind_of(uint8_t op)
{
    if (op >= 0x28 && op <= 0x3E)
        return (SCAN_MEMARG);
    if (op >= 0x45 && op <= 0xC4)
        return (SCAN_NONE);
    return ((enum scan_kind)SCAN_KINDS[op]);
}

enum scan_kind scan_kind_of_fc(uint32_t op)
{
    return (op < 18 ? (enum scan_kind)SCAN_KINDS_FC[op] : SCAN_BAD);
}

enum scan_kind scan_kind_of_fd(uint32_t op)
{
    if (op <= 0x0B || op == 0x5C || op == 0x5D)
        return (SCAN_MEMARG);
    if (op == 0x0C || op == 0x0D)
        return (SCAN_V128);
    if (op >= 0x15 && op <= 0x22)
        return (SCAN_LANE);
    if (op >= 0x54 && op <= 0x5B)
        return (SCAN_MEMARG_LANE);
    return (op <= 0xFF ? SCAN_NONE : SCAN_BAD);
}
