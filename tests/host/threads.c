/*
** Copyright (C) 2026 BB-Wasm Owner
**
** This file is part of BB-Wasm.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/threads.c -- two systems that nap, in two modules that
** share nothing: on two threads they run at once, and on one, in turn.
**
** threads.md, *Threads*
*/

#include <stdio.h>
#include <time.h>

#include "cases.h"

