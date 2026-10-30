# src/wasm_inside.h

The wasm backend's files share this private header.
It pulls in wasmtime, sets the constants, numbers the engine kinds and the settings, and declares the backend structs and the shared helpers.

## The constants and the numbers

`WASM_PAGE` is 64 KiB, the page of wasm: the granule of every memory.
`WASM_FAR` is an epoch deadline that no call reaches until the watchdog arms a nearer one.
`WASM_FUEL` is the fuel of a metered store outside a call of a system: more than any call spends.
`WASM_OWN` is how many own memories, and how many own tables, a module can have: 1. `docs/intern/wasm_limits.c.md` tells why.
`WASM_NO_MEMORY` is the reason of a failed allocation: `out of memory`.

`enum wasm_engine` carries two bits, metered and interruptible, so a module needs one of four engines.
`enum wasm_key` numbers the seven settings, in the order that `docs/intern/wasm_settings.c.md` reads them.

## The structs

`wasm_settings_t` holds the values of the settings of the backend.
`backend` holds the four engines, the watchdog, the wake event, the lock, the armed call, the block being adopted, and the settings.
`backend_memory`, `backend_module`, `backend_instance` and `backend_func` hold an adopted block, a compiled module, a live instance, and a kept function.
A module keeps its import list and its export list, an instance keeps its bounds, and a kept function keeps the name of its export.
`wasm_own_t` and `wasm_offered_t` hold a module's own memory and a wrapped host function.

## Functions

| Function | Result |
| --- | --- |
| `wasm_drop` and `wasm_fail` | Declared here, given by `docs/intern/wasm_fail.c.md`. |
| `wasm_read_settings` | Declared here, given by `docs/intern/wasm_settings.c.md`. |
| `wasm_make_own` | Declared here, given by `docs/intern/wasm_own.c.md`. |
| `wasm_adopt_into` | Declared here, given by `docs/intern/wasm_holder.c.md`. |
| `wasm_watch` and `wasm_start_watch` | Declared here, given by `docs/intern/wasm_engine.c.md`. |
| `wasm_get_engine` | Declared here, given by `docs/intern/wasm_adopted.c.md`. |
| `wasm_make_offered`, `wasm_make_types` and `wasm_call_offered` | Declared here, given by `docs/intern/wasm_offered.c.md`. |
| `wasm_set_limits` and `instance_set_work_limit` | Declared here, given by `docs/intern/wasm_limits.c.md`. |
| `wasm_find_func` | Declared here, given by `docs/intern/wasm_func.c.md`. |
