# src/wasm_scan_read.c

This file reads the numbers of the binary format: unsigned and signed LEB128, and a skip of bytes.

A number that runs past its bytes, or past 64 bits, marks the scan as bad.

## Functions

| Function | Result |
| --- | --- |
| `scan_u64` | Reads an unsigned LEB128 number of at most 64 bits. |
| `scan_u32` | Reads an unsigned LEB128 number, and marks the scan bad above 32 bits. |
| `scan_s64` | Reads a signed LEB128 number, and extends its sign. |
| `scan_skip` | Skips a count of bytes, or marks the scan bad if fewer remain. |
