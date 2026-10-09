# What was refused: Modules on the wasm backend

The record of `wasm.md`: `readme.md`, *Three layers, three seams*.

## A compiled module reaches them through accessors

Of `wasm.md`, *A compiled module reaches them through accessors*.

Refused: refusing every export `init` did not register, which would refuse the `memory` and
`__heap_base` a compiler emits unasked and make every toolchain strip to be loaded at all. The
condition that brings it back is an export becoming callable by something other than its own module.

Refused: a rewriter of our own doing the same substitution, which is Binaryen's job done twice. The
condition that brings it back is Binaryen being unable to inline or strip something this needs.

Refused: one memory per module with the shared and private blocks mapped into it, which makes a C
pointer enough and costs the TLB aliasing `costs.md`, *What we give up*, already declines. The
condition that brings it back is the opaque calls measurably costing the loop, or LLVM learning to
address a second memory, which retires the accessor module altogether.

## Exports

Of `wasm.md`, *Exports*.

Refused: leaving `reload` out to say "cannot", which is what these documents first had. It is a
statement made by an absence, and nothing can tell an export left out on purpose from one forgotten;
`capabilities.md` already holds that the absence of a guarantee is written down by somebody rather
than arrived at. No condition brings it back.

Refused: a `reload` that answers "no" when asked, which says it only by being tried - and on a
backend that cannot hold two instances the old one is shut down before the new one can answer. The
condition that brings that back is no backend taking that order.
