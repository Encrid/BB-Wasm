/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "adopt.h"

typedef struct {
    const char *word;
    size_t at;
    size_t size;
    int64_t value;
    const char *text;
} adopt_word_t;

static const adopt_in_t ADOPT_IN = {
    .block_len = 2 * ADOPT_PAGE, .guard = 64 << 20,
    .params = { BACKEND_I32, BACKEND_I64 }, .param_count = 2,
    .module = "bridge", .field = "ping", .store_field = "shared",
    .store_imports = 1, .stray = "stray"
};

static const adopt_word_t ADOPT_WORDS[] = {
    { "one-page", offsetof(adopt_in_t, block_len), 8, ADOPT_PAGE, nullptr },
    { "guard-32", offsetof(adopt_in_t, guard), 8, 32 << 20, nullptr },
    { "first-i64", offsetof(adopt_in_t, params[0]), 1, BACKEND_I64, nullptr },
    { "second-i32", offsetof(adopt_in_t, params[1]), 1, BACKEND_I32, nullptr },
    { "one-param", offsetof(adopt_in_t, param_count), 4, 1, nullptr },
    { "other", offsetof(adopt_in_t, module), sizeof(char *), 0, "other" },
    { "pong", offsetof(adopt_in_t, field), sizeof(char *), 0, "pong" },
    { "spare", offsetof(adopt_in_t, store_field), sizeof(char *), 0, "spare" },
    { "no-import", offsetof(adopt_in_t, store_imports), 4, 0, nullptr },
    { "no-trap", offsetof(adopt_in_t, stray), sizeof(char *), 0, "write" }
};

constexpr size_t ADOPT_WORD_COUNT = sizeof(ADOPT_WORDS) / sizeof(*ADOPT_WORDS);

void adopt_set_input(adopt_t *t, const char *word)
{
    const adopt_word_t *w = ADOPT_WORDS;
    const void *from;

    t->in = ADOPT_IN;
    while (w < ADOPT_WORDS + ADOPT_WORD_COUNT && strcmp(w->word, word))
        w++;
    if (w == ADOPT_WORDS + ADOPT_WORD_COUNT)
        return;
    from = w->text ? (const void *)&w->text : (const void *)&w->value;
    memcpy((char *)&t->in + w->at, from, w->size);
}
