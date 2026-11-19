# src/wasm_scan_imports.c

This file reads the imports and the memories of a module, and gives each memory its role.

An import of `bridge.shared` or `bridge.private` is a memory of Bridge. An import of `main.memory` is the own memory of `acc.wasm`. A memory of the memory section is the own memory of the module.

## Functions

| Function | Result |
| --- | --- |
| `scan_add_memory` | Adds a memory with its role, or marks the scan bad past 16 memories. |
| `scan_imports` | Reads the imports: it counts the imported functions, and adds each imported memory. |
| `scan_memories` | Adds each memory that the module defines, with the role of its own memory. |
