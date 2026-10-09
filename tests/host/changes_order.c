/*
** Copyright (C) 2026 BB-Wasm Owner
**
** This file is part of BB-Wasm.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/changes_order.c -- two systems of one phase, in two
** modules, that attach one component to the same entities. Unordered, the
** second does not load. Ordered, the one stated to run last is the one
** whose value stays, whichever of them was loaded first.
**
** schedule.md, *Order within a phase is never implicit*
*/

#include "cases.h"

