/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>

#include "host.h"

typedef struct host_read {
    struct host_read *next;
} host_read_t;

static host_read_t *read_blocks;

static void *read_whole(FILE *f, size_t *len)
{
    host_read_t *block;

    fseek(f, 0, SEEK_END);
    *len = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);
    block = malloc(sizeof(*block) + *len);
    if (block && fread(block + 1, 1, *len, f) != *len) {
        free(block);
        block = nullptr;
    }
    fclose(f);
    if (!block)
        return (nullptr);
    block->next = read_blocks;
    read_blocks = block;
    return (block + 1);
}

void *host_read_module(const char *file, size_t *len)
{
    char path[1024];
    FILE *f;
    void *data = nullptr;

    snprintf(path, sizeof(path), "%s/%s.wasm", MODULE_DIR, file);
    f = fopen(path, "rb");
    if (f)
        data = read_whole(f, len);
    if (data)
        return (data);
    printf("cannot read %s\n", path);
    exit(2);
}

void host_free_modules(void)
{
    host_read_t *next;

    while (read_blocks) {
        next = read_blocks->next;
        free(read_blocks);
        read_blocks = next;
    }
}
