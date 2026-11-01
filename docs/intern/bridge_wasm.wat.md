# src/bridge_wasm.wat

This file holds the accessors a compiled module reaches Bridge's two other memories through.
A wasm instruction names the memory it loads, and no C compiler emits a load from a memory that is not the module's own.
So the accessors are written by hand in wasm text, where each load and store names its memory.

## The three memories

The module imports three memories: the shared world data, its own private data, and its own linear memory.
A module holds no shared address: it states a coordinate, and an accessor turns the coordinate into a bounded offset.
The document of the header `bridge_abi.h` of Bridge gives the capability table that the accessors read.

## resolve

`$resolve` maps a coordinate to an offset in the shared memory, or it traps.
It takes a slot, a packed location, a field, a width, and a flag that tells a store from a load.
`docs/extern/bridge_wasm.h.md`, *The macros* packs the location from a table and a row, or from an id.
`$resolve` reads the capability table from the private header, and branches on the `kind` of the slot:

1. It traps if the slot is past the declared types, or a store names a read-only type.
2. It traps if the field and the width run past the stride of the type.
3. For a table or a toggle, it traps unless the row is below the row count. Then it returns the column base plus the row by the stride.
4. For a singleton, it returns the kept cell of the slot.
5. For a sparse set, it reads the cell for the id, and traps when the cell is 0.
6. For a direct-indexed type, it tests the presence bit of the id, then takes the cell.

Each branch adds the field, and each one traps when a bound does not hold.
The header points at the row counts and the blocks of the tables, which Bridge keeps in `$shared`.

## The accessors

Each accessor below builds on `$resolve` or reads the header the same way:

- `acc_get_*` and `acc_set_*` resolve by width, then read or write the shared memory. A store sets the flag, so a read-only type traps.
- `acc_get_location` gives the table and the row of an id, or all ones when the id is dead. `acc_get_entity` gives the id of a row, from the block of its table.
- `acc_get_rows` gives the row count of a table. It traps when the id is past the tables of the header.
- `acc_has` tells whether a sparse, direct-indexed, or singleton slot holds a value for an id.
- `acc_is_on` and `acc_set_toggle` read and write the presence bit of a row of a toggle.
- `acc_get_own_32`, `acc_get_own_64` and `acc_set_own_32` reach the private memory. `acc_set_own_32` writes 4 bytes, and traps unless all 4 stay below the published table and leave the header alone.
- `acc_copy_to_private` copies into the scratch region, below the published table.
- `acc_copy_to_shared` copies into the shared memory, and stays inside one owned range. `acc_copy_from_shared` copies out, inside the arena bounds.

## The build

The module build merges this file into the compiled module as `acc`, inlines every accessor into its callers, and strips the exports.
Before the merge, `tools/wasm_guard.py` checks the imports of the compiled module: `docs/tools/wasm_guard.py.md`.

The compile takes the setting `simd` of `.toolchain/config/variables.toml`, which is `-msimd128`, and then the `flags` of the module.
So `-mno-simd128`, as the setting or in the `flags` of one module, turns SIMD off.
The merge and the optimizer take `--enable-simd` in `features`, so binaryen keeps the SIMD instructions that the compile made.
