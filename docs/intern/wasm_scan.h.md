# src/wasm_scan.h

The types and the functions of the check at load: `docs/intern/wasm_scan_check.c.md`.
They hold the reader of the binary format of wasm, the walk over the instructions, and the match of the accessors.

A role tells what a memory of the module is:

| Role | Memory |
| --- | --- |
| `SCAN_SHARED` | The import `bridge.shared`. |
| `SCAN_PRIVATE` | The import `bridge.private`. |
| `SCAN_OWN` | A memory that the module defines, or the import `main.memory` of `acc.wasm`. |
| `SCAN_OTHER` | Each other memory. |

A kind tells which immediates an opcode has, so the walk knows how to read them.
