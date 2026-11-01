# include/bridge_wasm_acc.h

This header declares the host calls a module imports and the accessors that reach its two other memories.
`docs/intern/bridge_wasm.wat.md` gives the accessors; this header names them for C.

## The imports and the macros

`BRIDGE_IMPORT`, `BRIDGE_ACC` and `BRIDGE_EXPORT` mark a function as a host call, an accessor, or a module export.
The host calls are the load-time and phase entries: declare, add_system, observe, spawn, destroy, attach, detach and alloc.
`BRIDGE_HEADER` reads a field of the private header by its offset.

## Functions

| Function | Result |
| --- | --- |
| `acc_get_*` and `acc_set_*` | Get and set a cell of the shared memory, by width and sign. |
| `acc_get_location` | Gives the location of an id, packed in 64 bits, or all ones for a dead id. |
| `acc_get_entity` | Gives the id that owns a table and a row. |
| `acc_get_rows` | Gives the row count of a table, from the table directory. |
| `acc_has` | Gives 1 when a slot kept outside a table is present for an id. |
| `acc_is_on` and `acc_set_toggle` | Read and set a row's bit in a toggle. |
| `acc_get_own_32`, `acc_get_own_64` and `acc_set_own_32` | Get and set a value of the private memory. |
| `acc_copy_to_private`, `acc_copy_to_shared` and `acc_copy_from_shared` | Copy bytes between the module's own memory and the other two. |
