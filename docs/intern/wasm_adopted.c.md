# src/wasm_adopted.c

This file builds the wasmtime engines and routes every linear memory wasmtime asks for.
An engine either wraps a block Bridge adopted or hands the module a fresh own memory.

## Making an engine

`make_engine` configures one engine for a kind: fuel on when metered, epoch interruption on when timed.
It takes the reservation, the guard, `max_wasm_stack`, `opt_level` and `parallel_compilation` of the settings.
It sets the async stack of wasmtime to `max_wasm_stack` too: `validate` of wasmtime 48.0.5 refuses a `max_wasm_stack` above it, and its default is 2 MiB.
The backend runs no async call, so the async stack has no other use.

It turns on multiple memories, since a module imports the two memories of Bridge beside its own one.
It forbids a memory to move, since an adopted block stays where Bridge put it, and an own memory grows only inside its reservation.
It turns off the copy on write of a memory image, since each memory comes from the memory creator and not from wasmtime.
Then it sets the memory creator.
`wasm_get_engine` makes the engine for a kind on first need and keeps it.

## Routing a memory

`make_memory` is the memory creator wasmtime calls.
It refuses a request for more address space than `memory_get_shape` states.
With no block to adopt it calls `wasm_make_own`.
With a block it checks the size matches the import, then wraps the block through `get_adopted`, which never grows.

## Functions

| Function | Result |
| --- | --- |
| `wasm_get_engine` | Gives the engine for a kind, and makes it on first need. |
