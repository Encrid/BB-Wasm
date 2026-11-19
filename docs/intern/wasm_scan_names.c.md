# src/wasm_scan_names.c

This file reads a byte, a name and the limits of a memory or a table.

A name is a length and that many bytes. The file keeps a pointer into the module, and copies nothing.

## Functions

| Function | Result |
| --- | --- |
| `scan_byte` | Reads 1 byte, or marks the scan bad at the end. |
| `scan_name` | Reads a name. |
| `scan_name_is` | Tells if a name is a text. |
| `scan_limits` | Skips the limits: a flag, the least size, and the most size if the flag has it. |
