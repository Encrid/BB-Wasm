# src/wasm_scan_job.c

This file makes and frees the job of 1 check: the trusted accessors, their canonical forms, and the module.

The accessors come from the copy of `acc.wasm` that the backend holds: `docs/intern/wasm_scan_acc.c.md`.

## Functions

| Function | Result |
| --- | --- |
| `scan_load` | Reads the accessors and makes their forms, then reads the module and makes its memos and the room of its frames. |
| `scan_free_job` | Frees each form, each memo, the frames and each list of bodies. |
