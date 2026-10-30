# src/wasm_func.c

This file finds the exported functions of a module in the types of the module, with no instance.
The instance comes at the first call: `docs/intern/wasm_call.c.md`.

## Finding an export

`find_export` looks for the export of a name in the export list that `module_compile` kept.
`is_shaped` tells if a function type is `() -> i32`.
`check_export` refuses an export that is no function of that shape, and writes why when `err` gives room for it.
`instance_get_func` keeps a function of that name and shape, with the name of its export in the list.
If the instance is there, it finds the function in it at once.
`wasm_find_func` reads the export of the kept name from the instance, keeps its function, and deletes the extern.

## Functions

| Function | Result |
| --- | --- |
| `instance_get_func` | Gives a kept exported `() -> i32` function, or null with `err`. |
| `wasm_find_func` | Finds a kept function in the instance. |
