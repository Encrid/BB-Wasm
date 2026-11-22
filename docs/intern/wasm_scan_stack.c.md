# src/wasm_scan_stack.c

This file matches a function and each function that it calls, with a stack of frames in place of a recursion of the C stack.

A frame holds the walk of 1 body: its place in the body, its depth of blocks and its canonical form.
When a walk meets a call to a function whose match is not known yet, it stops before that call: `scan_resume` gives `SCAN_WAITS`.
`scan_match_from` then starts a frame for the callee on top of the stack. When the callee is matched, the walk of the caller goes on from that call.
So each callee is matched before the form of its caller holds it, in the order of the calls, as a recursion would do it.

A function on the stack is `SCAN_BUSY`, so a cycle of calls matches nothing, and the stack never holds a function 2 times.
Thus the stack holds at most 1 frame for each defined function. `scan_load` takes room for that many frames, so a long chain of calls needs no more room: `docs/intern/wasm_scan_job.c.md`.

A walk that fails marks the job `broken`. A form that finds no memory marks the job `no_memory`.

## Functions

| Function | Result |
| --- | --- |
| `find_acc` | Gives the accessor whose form equals a form, or -1. |
| `start` | Starts the frame of a function, marks it busy, and walks its body up to its first unknown callee. |
| `close_frame` | Keeps the match of a function and whether it touches a memory of Bridge, and frees its form. |
| `scan_match_from` | Matches a function and each callee that it reaches. |
