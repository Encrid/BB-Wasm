# src/wasm_backend.c

This file opens and closes the backend, states its guarantees, and reports the memory shape.

## What the backend guarantees

`backend_get_capabilities` answers a sandbox with a work limit, a timeout, a memory limit, an element limit, reload, and side-by-side loading.
It runs a module as wasm in wasmtime, so it can bound work, time, memories and tables, and hold two libraries at once.
The header `bridge_backend.h` of Bridge lists what Bridge asks each backend for.

## Starting and stopping

`backend_create` allocates the backend, reads its settings with `wasm_read_settings`, and starts the watchdog with `wasm_start_watch`.
If a step fails, it frees the backend and gives null, with the reason in `err`. A failed allocation gives `out of memory`.
It builds no engine here; `docs/intern/wasm_adopted.c.md` makes each engine on first need.
`backend_destroy` sets the quit flag, wakes the watchdog, joins it, and deletes every engine.

## Functions

| Function | Result |
| --- | --- |
| `backend_create` | Gives a new backend, or null with the reason in `err`. |
| `backend_destroy` | Stops the watchdog, deletes every engine, and frees the backend. |
| `backend_get_capabilities` | Gives the flags the backend keeps: work limit, timeout, memory limit, element limit, reload, side by side. |
| `backend_get_abi` | Gives `BACKEND_ABI`. Bridge reads it before all else and rejects a mismatch. |
| `memory_get_shape` | Gives the reservation and the guard of the settings, and a 64 KiB granule. |
