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
