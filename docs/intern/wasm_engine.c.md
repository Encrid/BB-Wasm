# src/wasm_engine.c

This file holds the watchdog thread that interrupts a call which runs past its timeout.
The thread sleeps until a timed call arms it, then wakes at the call's deadline.

## The watchdog

`read_armed` copies the armed call under the lock: its number, its deadline, and the quit flag.
With no armed call the thread waits on the wake event with no deadline.
With an armed call `wait_until` sleeps until the deadline, or until the wake event fires first.
It remembers the interrupted call, so it does not interrupt the same call twice.

When the deadline passes, `interrupt` takes the lock again. It increments the epoch of the engine only if the call that it read is still armed.
The increment traps the running call.

Without that check, a call can end at its deadline, and the next timed call can arm the same engine. Then a late increment traps that next call.
A call ends with the lock, when it disarms. So the next call sets its deadline after each increment for the call before it.

## The case of a late raise

The case `instance_set_timeout-the-call-after-a-late-deadline-not-interrupted` of `tests/cases/wasm.toml` checks this rule.
It loads the module `spinfirst` with an init timeout of 1 ns. Then it loads `spinlong`, with an init timeout of 20 s.
Both are `tests/modules/spinlong.wat`, whose init loops for about 2 s.

Its `hold` keeps the 1st `os_get_now` of this file, in `wait_until`, until 4 calls of `wasmtime_context_set_epoch_deadline`.
The 4th call is the deadline of the init of `spinlong`. So the watchdog reads the time of `spinfirst` only after the next call is armed.

The fixed watchdog then finds that `spinfirst` is no longer armed, and does not raise. This verdict does not depend on the time.
The old watchdog raised, and the init of `spinlong` trapped with `interrupt`, but only if that raise came before the end of `spinlong`.

The old code fails only when 2 windows hold, and each of them is a bound of time.

- The watchdog must read the time of `spinfirst` while it is armed, or the hold keeps a later read. So it must wake within the 2 s of `spinfirst`.
- After the release, the old watchdog must raise before `spinlong` ends. So it must run within the 2 s of `spinlong`.

No hold can close the 2nd window, since the loop of `spinlong` runs in wasm, where the layer sees no call.
The measures on the old code: with a loop of about 90 ms, 1 of 120 copies passed. With `spinlong` at 2 s and a short 1st init, 2 of 120 passed, by the 1st window.
With both inits at about 2 s, 0 of 120 passed, and the fix passed 20 of 20 rounds.

## Starting the watchdog

`wasm_start_watch` makes the wake event and the lock, then starts the thread on a stack of `watchdog_stack` bytes.
If one of the three fails, it frees the event and the lock that it made.
Then it gives -1, with the reason of the watchdog in `err`: `readme.md`, *Messages*.

## Functions

| Function | Result |
| --- | --- |
| `interrupt` | Increments the epoch of the engine if a call is still the armed one, and gives that call. |
| `wasm_watch` | Runs the watchdog loop until the backend quits. |
| `wasm_start_watch` | Starts the watchdog thread and gives 0, or gives -1 with `err`. |
