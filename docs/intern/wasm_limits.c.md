# src/wasm_limits.c

This file sets the limits of an instance: its work, its time, its memories and its reference tables.

## The limits

`instance_set_work_limit` sets the fuel of a metered module, and a module that is not metered ignores it.
`instance_set_timeout` keeps the nanoseconds of the next call.
`instance_set_memory_limit` and `instance_set_element_limit` keep their bound in the instance, then call `wasm_set_limits`.
`wasm_set_limits` gives all the bounds of the instance to its store at once, since wasmtime takes them only together.

| Bound of the store | Value |
| --- | --- |
| The bytes of each memory | `memory_limit`, or no bound. |
| The elements of each table | `element_limit`, or no bound. |
| The instances | The bound of wasmtime, 10000. The backend makes each instance of the store itself. |
| The tables | `WASM_OWN`: 1. |
| The memories | The lent memories, and `WASM_OWN`. |

wasmtime takes the bound of the bytes and the bound of the elements for each memory and each table, and not for their sum.
Its header `wasmtime/store.h` of wasmtime 48.0.5 says so at line 91: "the maximum number of elements in a table".
Thus one own memory and one own table make `memory_limit` and `element_limit` bounds of the whole module.

The bound of the memories also counts the lent memories, since wasmtime counts each memory that an instance of the store defines.
The source is `bump_resource_counts` in the file `crates/wasmtime/src/runtime/store.rs` of wasmtime 48.0.5.
Each lent memory comes from a holder, which is an instance in the store of the module: `docs/intern/wasm_holder.c.md`.
Thus a module has one own memory beside the lent ones, and a second own memory or table fails its instantiation.

## Functions

| Function | Result |
| --- | --- |
| `instance_set_work_limit` | Sets the fuel that a metered call may spend. |
| `instance_set_timeout` | Keeps the nanoseconds that the next call may run. |
| `instance_set_memory_limit` | Keeps the bound of the bytes of a memory, and gives it to the store. |
| `instance_set_element_limit` | Keeps the bound of the elements of a table, and gives it to the store. |
| `wasm_set_limits` | Gives the bounds of the instance to its store. |
