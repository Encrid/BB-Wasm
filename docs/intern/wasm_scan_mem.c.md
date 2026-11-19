# src/wasm_scan_mem.c

This file reads the immediates that name a memory, and writes the role of the memory.

A memory of Bridge in a function marks that function as one that touches Bridge.

## Functions

| Function | Result |
| --- | --- |
| `scan_memarg` | Reads the alignment, the memory and the offset of a load or a store, and a lane if it has one. |
| `scan_memory` | Reads the memory of `memory.size`, `memory.grow` or `memory.fill`. |
| `scan_copy` | Reads the 2 memories of `memory.copy`. |
| `scan_init` | Reads the data segment and the memory of `memory.init`. |
