# src/wasm_scan_plain.c

This file reads the immediates that need no mapping: numbers, labels, types and raw bytes.

A constant keeps its value. A float and a lane keep their bytes.

## Functions

| Function | Result |
| --- | --- |
| `scan_none` | Reads no immediate. |
| `scan_raw` | Copies the bytes of a float, a vector, a lane or a reference type. |
| `scan_number` | Copies a constant, a label or an index as a number. |
| `scan_labels` | Copies the labels of a `br_table`. |
| `scan_types` | Copies the value types of a `select`. |
