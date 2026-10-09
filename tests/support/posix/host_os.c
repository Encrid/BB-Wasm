/*
** Copyright (C) 2026 BB-Wasm Owner
**
** This file is part of BB-Wasm.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/support/posix/host_os.c -- what a host program asks the operating
** system itself, and nothing of ours: on Linux and macOS.
*/

#include <dlfcn.h>
#include <pthread.h>
#include <stdlib.h>

#include "host.h"

static void *run_thread(void *arg)
{
    host_thread_t *t = arg;

    t->run(t->arg);
    return (nullptr);
}

/*
** host_thread_start
**
** Starts a thread that calls `t->run` with `t->arg`.
*/
void host_thread_start(host_thread_t *t)
{
    pthread_t *thread = malloc(sizeof(*thread));

    t->handle = thread;
    pthread_create(thread, nullptr, run_thread, t);
}

/*
** host_thread_join
**
** Waits for a thread to end.
*/
void host_thread_join(host_thread_t *t)
{
    pthread_t *kept = t->handle;
    pthread_t thread = *kept;

    pthread_join(thread, nullptr);
    free(kept);
}
