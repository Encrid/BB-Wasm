# src/wasm_scan_module.c

This file reads a whole module: its header, then each section that the check needs.

It reads the imports, the functions, the memories, the exports, the code and the data. It skips each other section.

## Functions

| Function | Result |
| --- | --- |
| `scan_read` | Reads a module. It gives false for bytes that are not a module of version 1, or that it cannot read. |
