/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
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

void host_thread_start(host_thread_t *t)
{
    pthread_t *thread = malloc(sizeof(*thread));
    pthread_attr_t attr;

    t->handle = thread;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, t->stack);
    pthread_create(thread, &attr, run_thread, t);
    pthread_attr_destroy(&attr);
}

void host_thread_join(host_thread_t *t)
{
    pthread_t *kept = t->handle;
    pthread_t thread = *kept;

    pthread_join(thread, nullptr);
    free(kept);
}
