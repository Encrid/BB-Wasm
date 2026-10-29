# include/bridge_wasm.h

A wasm module compiles this header, with `bridge_abi.h`, to reach Bridge and read its own data.
It pulls in the accessors, the scratch arena, and the readers for spans, entities, kept storage, toggles, observers and the log.
It adds the macros a module registers with, and reads and writes its cells through.

## The macros

`BRIDGE_COMPONENT` and `BRIDGE_DECLARE` register a component by its type and descriptor; `BRIDGE_DECLARE_TAG` registers a tag that carries no bytes.

| Macro | Parameters |
| --- | --- |
| `BRIDGE_DECLARE` | `kind`, `name`, `T`, `on_unload`, `storage`, `on_destroy` |
| `BRIDGE_COMPONENT` | `name`, `T`, `on_unload`, `storage`, `on_destroy`. The kind is `BRIDGE_STRUCT`. |
| `BRIDGE_DECLARE_TAG` | `name`, `on_unload`, `storage`, `on_destroy`. The kind is `BRIDGE_TAG`. |

`name` is a string literal, and `T` is a type with the descriptor `T##_desc`.
The last three parameters fill the fields of `bridge_declaration_t` with the same names.

`BRIDGE_ADD_SYSTEM` and `BRIDGE_OBSERVER` register a system and an observer, each with its terms, its changed slots and its `before`.
`before` holds the systems or the observers that run before it.
`BRIDGE_ATTACH` lends a value, then attaches it to a slot.
`BRIDGE_AT` packs a table and a row, or an id, into one place.
`BRIDGE_GET` and `BRIDGE_SET` read and write a field of a cell, and pick the width from its type.

`BRIDGE_PUT_SLOTS` stages a list of slots in the arena; `BRIDGE_LD_` and `BRIDGE_ST_` begin the load and store macros those two pick from.
`docs/extern/bridge_wasm_acc.h.md` describes the accessors these macros build on.
