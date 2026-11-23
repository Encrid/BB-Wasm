# src/wasm_scan_walk.c

This file walks the body of a function, and writes its canonical form.

The form holds the local declarations, with each run of 1 type merged, then each opcode and its immediates.
A memory becomes its role, and a call becomes the accessor that the callee matches. So the form does not depend on the numbering of a module.

A walk that waits goes back to the start of the call: its place and the length of its form are those before the opcode.
So when it goes on, it reads the call again, now with its callee matched.

## Functions

| Function | Result |
| --- | --- |
| `scan_immediate` | Reads the immediates of 1 kind with its handler. |
| `scan_begin` | Starts the walk of a body in a frame, reads its locals, and walks it as `scan_resume` does. |
| `scan_resume` | Walks a body to its last `end`: `SCAN_DONE`, or `SCAN_FAILED` if it cannot read it. It gives `SCAN_WAITS` before a call whose callee is not matched yet. |
