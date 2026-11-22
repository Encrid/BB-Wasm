/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_BACKENDS_WASM_SCAN_H
    #define BRIDGE_BACKENDS_WASM_SCAN_H

    #include <stddef.h>
    #include <stdint.h>

constexpr uint32_t SCAN_MEMORIES = 16;
constexpr uint32_t SCAN_FUNCTIONS = 65'536;
constexpr uint32_t SCAN_WHY = 200;
constexpr int32_t SCAN_UNKNOWN = -2;
constexpr int32_t SCAN_BUSY = -3;
constexpr uint32_t SCAN_NO_WAIT = UINT32_MAX;

enum scan_role : uint8_t {
    SCAN_OTHER = 0,
    SCAN_SHARED = 1,
    SCAN_PRIVATE = 2,
    SCAN_OWN = 3
};

enum scan_kind : uint8_t {
    SCAN_BAD = 0,
    SCAN_NONE,
    SCAN_BLOCK,
    SCAN_LABEL,
    SCAN_LABELS,
    SCAN_CALL,
    SCAN_CALL_INDIRECT,
    SCAN_INDEX,
    SCAN_MEMARG,
    SCAN_MEMORY,
    SCAN_I32,
    SCAN_I64,
    SCAN_F32,
    SCAN_F64,
    SCAN_TYPES,
    SCAN_REF_TYPE,
    SCAN_PREFIX_FC,
    SCAN_PREFIX_FD,
    SCAN_INIT,
    SCAN_COPY,
    SCAN_TWO,
    SCAN_V128,
    SCAN_LANE,
    SCAN_MEMARG_LANE,
    SCAN_KIND_COUNT
};

typedef struct {
    const uint8_t *at;
    const uint8_t *end;
    bool bad;
} scan_t;

typedef struct {
    const uint8_t *at;
    uint32_t len;
} scan_name_t;

typedef struct {
    const uint8_t *at;
    uint32_t len;
} scan_body_t;

typedef struct {
    uint8_t *bytes;
    size_t len;
    size_t cap;
    bool bad;
} scan_out_t;

typedef struct {
    enum scan_role roles[SCAN_MEMORIES];
    uint32_t memories;
    uint32_t imported;
    uint32_t defined;
    scan_body_t *bodies;
    uint32_t refused;
    bool no_memory;
    char why[SCAN_WHY];
} scan_module_t;

enum scan_step : uint8_t {
    SCAN_FAILED = 0,
    SCAN_DONE,
    SCAN_WAITS
};

typedef struct scan_frame scan_frame_t;

typedef struct {
    scan_module_t acc;
    scan_module_t m;
    scan_out_t *canons;
    int32_t *match;
    bool *touched;
    scan_frame_t *frames;
    bool broken;
    bool no_memory;
} scan_job_t;

typedef struct {
    scan_job_t *job;
    const scan_module_t *m;
    scan_out_t *out;
    bool touches;
    uint32_t wait;
} scan_walk_t;

struct scan_frame {
    scan_walk_t w;
    scan_out_t out;
    scan_t s;
    uint32_t depth;
    uint32_t function;
};

typedef bool (*scan_handler_t)(scan_walk_t *w, scan_t *s,
    enum scan_kind kind);
typedef void (*scan_section_t)(scan_module_t *m, scan_t *s);

uint64_t scan_u64(scan_t *s);
uint32_t scan_u32(scan_t *s);
int64_t scan_s64(scan_t *s);
void scan_skip(scan_t *s, size_t n);

uint8_t scan_byte(scan_t *s);
void scan_name(scan_t *s, scan_name_t *name);
bool scan_name_is(const scan_name_t *name, const char *text);
void scan_limits(scan_t *s);

void scan_put(scan_out_t *o, const void *bytes, size_t n);
void scan_put_u64(scan_out_t *o, uint64_t value);
void scan_refuse(scan_module_t *m, const char *head, uint32_t index,
    const char *tail);
void scan_free(scan_module_t *m);

void scan_add_memory(scan_module_t *m, scan_t *s, enum scan_role role);
void scan_imports(scan_module_t *m, scan_t *s);
void scan_memories(scan_module_t *m, scan_t *s);

bool scan_is_bridge(const scan_module_t *m, uint32_t memory);
void scan_exports(scan_module_t *m, scan_t *s);
void scan_datas(scan_module_t *m, scan_t *s);

void scan_functions(scan_module_t *m, scan_t *s);
void scan_codes(scan_module_t *m, scan_t *s);
bool scan_read(scan_module_t *m, const uint8_t *bytes, size_t len);

enum scan_kind scan_kind_of(uint8_t op);
enum scan_kind scan_kind_of_fc(uint32_t op);
enum scan_kind scan_kind_of_fd(uint32_t op);

bool scan_immediate(scan_walk_t *w, scan_t *s, enum scan_kind kind);
bool scan_begin(scan_frame_t *f, const scan_body_t *body);
enum scan_step scan_resume(scan_frame_t *f);

bool scan_none(scan_walk_t *w, scan_t *s, enum scan_kind kind);
bool scan_raw(scan_walk_t *w, scan_t *s, enum scan_kind kind);
bool scan_number(scan_walk_t *w, scan_t *s, enum scan_kind kind);
bool scan_labels(scan_walk_t *w, scan_t *s, enum scan_kind kind);
bool scan_types(scan_walk_t *w, scan_t *s, enum scan_kind kind);

bool scan_block(scan_walk_t *w, scan_t *s, enum scan_kind kind);
bool scan_call(scan_walk_t *w, scan_t *s, enum scan_kind kind);
bool scan_two(scan_walk_t *w, scan_t *s, enum scan_kind kind);
bool scan_prefix(scan_walk_t *w, scan_t *s, enum scan_kind kind);

bool scan_memarg(scan_walk_t *w, scan_t *s, enum scan_kind kind);
bool scan_memory(scan_walk_t *w, scan_t *s, enum scan_kind kind);
bool scan_copy(scan_walk_t *w, scan_t *s, enum scan_kind kind);
bool scan_init(scan_walk_t *w, scan_t *s, enum scan_kind kind);

bool scan_load(scan_job_t *job, const uint8_t *bytes, size_t len);
void scan_free_job(scan_job_t *job);
const uint8_t *scan_acc(size_t *len);

int32_t scan_matched(const scan_walk_t *w, uint32_t function);
void scan_match_from(scan_job_t *job, uint32_t root);
const char *scan_refused(const uint8_t *bytes, size_t len, char *why,
    size_t size);

#endif
