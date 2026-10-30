# src/wasm_module.c

On this backend a module is compiled wasm, so its image is a wasmtime module.
This file compiles the bytes, lists the imports the module declares, and frees them.

## Compiling

`module_compile` allocates the image, then reads the demands on the source.
A work limit asks for a metered engine, and a timeout asks for an interruptible one.
It compiles the bytes with that engine, and keeps the import list and the export list.
Empty bytes, or bytes wasmtime cannot compile, are refused with a reason in `err`.
An image that it cannot allocate gives `out of memory`.
`module_list_imports` writes each import's namespace, field and kind, and gives the total count.

## Functions

| Function | Result |
| --- | --- |
| `module_compile` | Compiles the bytes for the engine the demands pick. Gives the module, or null with `err`. |
| `module_list_imports` | Writes up to `max` imports and gives how many the module declares. |
| `module_destroy` | Deletes the import list, the export list and the module, then frees the image. |
