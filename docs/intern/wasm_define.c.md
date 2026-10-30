# src/wasm_define.c

This file defines on the linker each host function that a module imports.

## Defining a function

`define_func` refuses a function after the first call, which makes the instance, or a signature wider than 16 parameters or 4 results.
The wrap keeps the kinds in arrays of 16 and 4, and each entry of Bridge has fewer parameters and results.
It wraps the function with `wasm_make_offered`, and `make_type` builds its type.
Then it defines the import on the linker, with `wasm_call_offered` as the function that wasmtime calls.
The linker frees the wrap when the store goes, and also when the definition fails.

## Functions

| Function | Result |
| --- | --- |
| `define_func` | Defines a host function on the linker, and gives 0. Gives -1 with `err` for a function that is too wide or too late, or that it cannot wrap. |
