# src/wasm_scan_check.c

This file is the check at load: only the accessors of `acc.wasm` touch the memories of Bridge.

A function that touches a memory of Bridge must have the canonical form of 1 function of `acc.wasm`. A call that it makes must go to such a function too.
The match of each function is kept, so each body is walked 1 time. A cycle of calls matches nothing.

The check refuses a function that touches a memory of Bridge and matches no accessor, or a function that it cannot read. It also refuses each module that the reader refuses, and it fails when an allocation fails.
`readme.md`, *Messages*, gives each reason.

## Functions

| Function | Result |
| --- | --- |
| `scan_matched` | Gives the accessor that a defined function matches, or -1. |
| `scan_refused` | Checks a module, and gives the reason of a refusal, or null. |
