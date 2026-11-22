# src/wasm_scan_flow.c

This file reads the immediates of the blocks, the calls and the prefixes.

A call to an imported function becomes that import. A call to a defined function becomes the accessor that it matches, or a mark of no match.
If that match is not known yet, `scan_call` writes the callee in `wait` of the walk, and gives false: the walk then waits.

## Functions

| Function | Result |
| --- | --- |
| `scan_block` | Copies the type of a block. |
| `scan_call` | Writes the callee of a call as an import, an accessor, or no match, or waits for its match. |
| `scan_matched` | Gives the accessor that a defined function matches, -1, or `SCAN_UNKNOWN` before its match. |
| `scan_two` | Copies 2 indices. |
| `scan_prefix` | Reads the opcode after a prefix, and its immediates. |
