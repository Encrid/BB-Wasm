# include/bridge_wasm_kept.h

Some components live outside the table rows: a singleton, a sparse set, or a direct-indexed column.
This header asks the accessors about them, and tells from the private header whether a table holds a type.

## Functions

| Function | Result |
| --- | --- |
| `bridge_has` | Gives whether a kept component is present for an id. |
| `bridge_has_column` | Gives 1 when a table holds a type, from `column_base`, and 0 when it does not. A tag counts. |
