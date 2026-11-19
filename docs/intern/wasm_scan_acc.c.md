# src/wasm_scan_acc.c

This file holds the bytes of the accessors, `acc.wasm`, embedded in the library at build with `#embed`.

So no file on the disk can stand in for the accessors that the check trusts. The artifact `bridge_wasm` reads `acc.wasm` as an input.

## Functions

| Function | Result |
| --- | --- |
| `scan_acc` | Gives the bytes of `acc.wasm` and their count. |
