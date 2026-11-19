# src/wasm_scan_exports.c

This file reads the exports and the data segments of a module.

A module that exports a memory of Bridge is refused, and so is an active data segment on a memory of Bridge.
`readme.md`, *Messages*, gives each reason.

## Functions

| Function | Result |
| --- | --- |
| `scan_is_bridge` | Tells if a memory is `bridge.shared` or `bridge.private`. |
| `scan_exports` | Reads the exports, and refuses an export of a memory of Bridge. |
| `scan_datas` | Reads the data segments, and refuses an active one on a memory of Bridge. |
