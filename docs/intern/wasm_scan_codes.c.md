# src/wasm_scan_codes.c

This file reads the function section and the code section of a module.

It keeps where the body of each function begins, and its length. More than 65,536 functions, or a code section with another count than the function section, mark the scan as bad.

## Functions

| Function | Result |
| --- | --- |
| `scan_functions` | Reads the count of the defined functions, and makes the list of their bodies. |
| `scan_codes` | Reads the body of each defined function. |
