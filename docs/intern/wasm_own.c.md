# src/wasm_own.c

This file gives a module its own linear memory: a reservation of `reservation` bytes, with `guard` bytes after it.
wasmtime calls three callbacks to read the memory, grow it, and free it.

## The callbacks

`get_own` reports the committed size, the reserved capacity, and the base.
`grow_own` commits more pages up to the reservation; past it, or on a failed commit, it refuses.
`free_own` releases the whole reservation and guard, then frees the record.

`wasm_make_own` allocates the record, reserves the address space, commits the minimum, and fills the callbacks.
A failed allocation gives `out of memory`.
A memory that starts past the reservation gets no address space.
With no address space, it frees the record and gives an error.

## Functions

| Function | Result |
| --- | --- |
| `wasm_make_own` | Reserves a module's own memory and gives wasmtime the callbacks that manage it. |
