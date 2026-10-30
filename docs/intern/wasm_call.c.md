# src/wasm_call.c

This file runs an exported function, and makes the instance of the module at its first call.
A timed call arms the watchdog before it runs and disarms it after.

## A call

`arm` takes the timeout that `instance_set_timeout` kept, and clears it.
For a timed call it sets the epoch deadline 1 tick ahead, records the call under the lock, and wakes the watchdog.
The watchdog adds that 1 tick at the end of the timeout, so the call stops then. With no timeout, the deadline is `WASM_FAR`.
`disarm` clears the active call and wakes the watchdog again.

`instantiate` links the module at the first call. Thus the start function of the module runs in the armed call, under its timeout.
It then finds each function that `instance_get_func` kept before: `docs/intern/wasm_func.c.md`.
An instance that it cannot make fails the call, with `made no instance` and the reason in `err`.
`run` runs the function with no argument, reads its one `i32` status, and reports a trap through `err`.

`call` first reads how many bytes of stack the thread has left, with `os_thread_stack_left` of Bridge-OS.
With less than `max_wasm_stack` and `stack_margin` together, it refuses the call before any wasm runs.
The reason names the bytes that are left and the bytes that the two need.
Bridge-OS gives 0 if the system does not tell the stack. Since `max_wasm_stack` is 1 or more, `call` then refuses too.
Source: the row `os_thread_stack_left` of the document of `os.h` of Bridge-OS.

wasmtime bounds only the wasm frames, from the first one down: `max_wasm_stack` in `wasmtime/config.h` of wasmtime 48.0.5.
`stack_margin` holds the other frames: those of `call` and of wasmtime up to the first wasm frame, and an offered function at the deepest one.
Thus no offered function checks the stack again.
A deep call on a thread with a small stack fails as a call, and the stack of the thread never overflows.

With enough stack, `call` arms, instantiates, runs and disarms. Then it gives a metered instance its fuel of `WASM_FUEL` again.
Thus a call that is no system, for example `shutdown`, never starts with the fuel that a system left.

## Functions

| Function | Result |
| --- | --- |
| `call` | Runs an exported function and reports its `i32` status, or gives 1 with `err`. |
