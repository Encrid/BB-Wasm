# src/wasm_settings.c

This file reads the settings that the deployment gives to the backend: `readme.md`, *Settings*.

## Reading the settings

`place` puts each setting at the place of its key, and refuses a setting that does not have the kind of its key.
`find_key` gives the place of a key, and refuses a key that it does not know. A setting with no key has the key `''`.
`wasm_read_settings` places the settings, refuses the first key that is missing, then reads the seven values in the order of `enum wasm_key`.

`read_size` takes the size of each key from its least, in the table below.
`read_level` takes `none`, `speed` or `speed_and_size`, which are the levels of `wasmtime_opt_level_t` in their order.
The flag `parallel_compilation` needs no check, since each of its two values is valid.
The first refusal stops the reading, with its reason in `err`.

## The least of each value

The backend refuses a value only when it or wasmtime 48.0.5 cannot run with it. Each least has its source:

| Key | Least | Why |
| --- | --- | --- |
| `reservation` | 0 | wasmtime gives the value as it is to the memory creator, and an own memory then starts and grows only inside it. Sources: `memory_reservation` in `crates/wasmtime/src/config.rs`, and `MemoryTunables` in `crates/environ/src/tunables.rs`. |
| `guard` | 0 | wasmtime takes any guard, and with 0 it checks each access. Source: `memory_guard_size` in `crates/wasmtime/src/config.rs`. |
| `max_wasm_stack` | 1 | `validate` in `crates/wasmtime/src/config.rs` refuses 0: "max_wasm_stack size cannot be zero". Its C API stops the process on that error, since `wasm_engine_new_with_config` unwraps it. |
| `stack_margin` | 0 | Any number of bytes is a margin. With 0, a call keeps no stack for the frames that are not wasm. |
| `watchdog_stack` | 0 | Bridge-OS starts a thread on at least the bytes that it gets, and rounds the size up to its least stack. Source: the row `os_thread_start` of the document of `os.h` of Bridge-OS. |

A negative number is no number of bytes, thus each key refuses it.

## Functions

| Function | Result |
| --- | --- |
| `wasm_read_settings` | Fills the settings of the backend, and gives 0. Gives 1, with the reason in `err`, for a setting that it refuses. |
