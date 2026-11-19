# src/wasm_scan_out.c

This file writes the canonical form of a function, and keeps the reason of a refusal.

The canonical form is a growing array of bytes. A number in it takes 8 bytes, so 2 encodings of 1 number give the same form.

## Functions

| Function | Result |
| --- | --- |
| `scan_put` | Adds bytes to the form, and grows its array. |
| `scan_put_u64` | Adds a number as 8 bytes. |
| `scan_refuse` | Keeps the first reason of a refusal, with the index of what it names. |
| `scan_free` | Frees the list of the bodies of a module. |
