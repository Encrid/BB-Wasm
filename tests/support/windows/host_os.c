/*
** Copyright (C) 2026 BB-Wasm Owner
**
** This file is part of BB-Wasm.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/support/windows/host_os.c -- what a host program asks the
** operating system itself, and nothing of ours: on Windows.
*/

#include <windows.h>

#include "host.h"

static DWORD WINAPI run_thread(LPVOID arg)
{
    host_thread_t *t = arg;

    t->run(t->arg);
    return (0);
}

/*
** host_thread_start
**
** Starts a thread that calls `t->run` with `t->arg`.
*/
void host_thread_start(host_thread_t *t)
{
    t->handle = CreateThread(nullptr, 0, run_thread, t, 0, nullptr);
}

/*
** host_thread_join
**
** Waits for a thread to end.
*/
void host_thread_join(host_thread_t *t)
{
    WaitForSingleObject(t->handle, INFINITE);
    CloseHandle(t->handle);
}
