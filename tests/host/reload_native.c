/*
** Copyright (C) 2026 BB-Wasm Owner
**
** This file is part of BB-Wasm.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/reload_native.c -- a native module reloaded: the old
** instance is shut down before the new library is opened, so a reload that
** fails leaves no code to keep.
**
** failure.md, *When something fails*
*/

#include <string.h>

#include "cases.h"

