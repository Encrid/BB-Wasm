# src/wasm_scan_kinds.c

This file gives the kind of the immediates of each opcode.

An opcode that the tables do not hold has the kind `SCAN_BAD`, and the check refuses its function.
The prefix `0xFC` holds the saturating conversions, the bulk memory and the tables. The prefix `0xFD` holds SIMD, with no relaxed SIMD.

## Functions

| Function | Result |
| --- | --- |
| `scan_kind_of` | Gives the kind of a 1-byte opcode. |
| `scan_kind_of_fc` | Gives the kind of an opcode after `0xFC`. |
| `scan_kind_of_fd` | Gives the kind of an opcode after `0xFD`. |
