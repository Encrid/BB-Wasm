/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "cases.h"

constexpr uint32_t TEST_CHAIN = 65'536;
constexpr size_t TEST_CHAIN_BYTES = 8 * TEST_CHAIN + 128;

static const uint8_t TEST_CHAIN_HEAD[] = { 0x00, 0x61, 0x73, 0x6d, 0x01,
    0x00, 0x00, 0x00, 0x01, 0x05, 0x01, 0x60, 0x00, 0x01, 0x7f };
static const uint8_t TEST_CHAIN_EXPORTS[] = { 0x07, 0x1c, 0x03, 0x04, 'i',
    'n', 'i', 't', 0x00, 0x00, 0x08, 's', 'h', 'u', 't', 'd', 'o', 'w', 'n',
    0x00, 0x01, 0x06, 'r', 'e', 'l', 'o', 'a', 'd', 0x00, 0x02 };

static size_t test_put_u32(uint8_t *to, uint32_t value)
{
    size_t n = 0;

    do {
        to[n] = (uint8_t)((value & 0x7f) | (value > 0x7f ? 0x80 : 0));
        value >>= 7;
        n++;
    } while (value);
    return (n);
}

static size_t test_put_body(uint8_t *to, uint32_t function)
{
    uint8_t code[8] = { 0x00, 0x41, 0x00, 0x0b };
    size_t len = 4;

    if (function >= 3 && function + 1 < TEST_CHAIN) {
        code[1] = 0x10;
        len = 2 + test_put_u32(&code[2], function + 1);
        code[len++] = 0x0b;
    }
    to[0] = (uint8_t)len;
    memcpy(&to[1], code, len);
    return (len + 1);
}

static size_t test_put_section(uint8_t *to, uint8_t id, const uint8_t *in,
    size_t len)
{
    size_t n = 1;

    to[0] = id;
    n += test_put_u32(&to[n], (uint32_t)len);
    memcpy(&to[n], in, len);
    return (n + len);
}

static size_t test_make_chain(uint8_t *to, uint8_t *work)
{
    size_t n = sizeof(TEST_CHAIN_HEAD);
    size_t len = test_put_u32(work, TEST_CHAIN);

    memcpy(to, TEST_CHAIN_HEAD, n);
    memset(&work[len], 0, TEST_CHAIN);
    n += test_put_section(&to[n], 0x03, work, len + TEST_CHAIN);
    memcpy(&to[n], TEST_CHAIN_EXPORTS, sizeof(TEST_CHAIN_EXPORTS));
    n += sizeof(TEST_CHAIN_EXPORTS);
    len = test_put_u32(work, TEST_CHAIN);
    for (uint32_t f = 0; f < TEST_CHAIN; f++)
        len += test_put_body(&work[len], f);
    return (n + test_put_section(&to[n], 0x0a, work, len));
}

void test_load_chain(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;
    uint8_t *bytes = malloc(TEST_CHAIN_BYTES);
    uint8_t *work = malloc(TEST_CHAIN_BYTES);

    if (bytes && work && !host_open(h)) {
        test_state_small(&a, identity, "ok");
        a.len = test_make_chain(bytes, work);
        a.bytes = bytes;
        host_load(h, &a);
        host_close(h);
    }
    free(bytes);
    free(work);
}
