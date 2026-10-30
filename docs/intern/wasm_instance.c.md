# src/wasm_instance.c

This file creates and destroys a module instance, and gives it the memory it imports.
An instance carries its own store, context and linker, and a list of the functions found in it.

## Opening a store

`instance_create` allocates the instance, and gives null with `out of memory` when it cannot.
`open_store` makes the store, the context and the linker for the module's engine.
It gives the store its first bounds with `wasm_set_limits`: no bound on a memory or a table, and one own memory: `docs/intern/wasm_limits.c.md`.
A metered module starts with the fuel of `WASM_FUEL`; a timed module starts with a deadline that never fires until a call arms it.
`give_memory` adopts an imported block into the instance and defines it on the linker by namespace and field.
The linker does not take the extern, so `give_memory` deletes it after the definition.

## Functions

| Function | Result |
| --- | --- |
| `instance_create` | Makes an instance and gives it every imported memory. Gives null with `err` on a failure. |
| `instance_destroy` | Frees the found functions, the linker and the store, then the instance. |
