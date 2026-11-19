/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "wasm_scan.h"

static const uint8_t SCAN_ACC[] = {
#embed "acc.wasm"
};

const uint8_t *scan_acc(size_t *len)
{
    *len = sizeof(SCAN_ACC);
    return (SCAN_ACC);
}
