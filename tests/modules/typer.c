/*
** Copyright (C) 2026 BB-Wasm Owner
**
** This file is part of BB-Wasm.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/typer.c -- declares a struct other modules'
** components use as a field's type, and nothing else.
*/

#include "bridge_wasm.h"
#include "vec2.h"
#include "wasm_idle.h"

/*
** init
**
** Declares the struct, under the name a field of its type looks it up by.
*/
BRIDGE_EXPORT(init) int init(void)
{
    BRIDGE_COMPONENT("game:vec2_t", vec2_t, BRIDGE_STRIP, BRIDGE_TABLE);
    return (0);
}
