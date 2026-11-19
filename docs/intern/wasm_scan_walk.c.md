# src/wasm_scan_walk.c

This file walks the body of a function, and writes its canonical form.

The form holds the local declarations, with each run of 1 type merged, then each opcode and its immediates.
A memory becomes its role, and a call becomes the accessor that the callee matches. So the form does not depend on the numbering of a module.

## Functions

| Function | Result |
| --- | --- |
| `scan_immediate` | Reads the immediates of 1 kind with its handler. |
| `scan_walk` | Walks a body to its last `end`, and gives false if it cannot read it. |
