# BB-Wasm

The wasm backend of Bridge: what runs a module that is a wasm binary, where it can reach
nothing it was not given. And what such a module is written with: its SDK.

It is a project of its own. It was written in Bridge's tree, and left it when Bridge kept only
its seams: `plan.md`, stage 0.

## What it is for

**Most modules.** A wasm module runs on every platform from one file, cannot read what it was
not handed, can be bounded in the work it does, in how long a call takes and in the memory it
has, and is ended where it faults with the deck still running. What needs hardware is the
native backend's, and nothing else is. `wasm.md` says how a module is written for this one,
and `wasmtime.md` what the engine behind it is asked to do.

## What it needs of Bridge

**Two packages, at one commit**: `bridge`, the library and the one header a host includes, and
`seams`, the three headers a backend and its SDK are written against. Nothing else of Bridge's
tree is read.

**The commit is pinned.** `bridge.txt`, at the root, states one commit of Bridge and the hash
of each archive Bridge published for it. A build fetches the two of its target from that
release, once, holds each against its hash before it is opened, and keeps them under `build/`:

    python build.py test

Bridge is not public, so the fetch is made with a token that reads it, handed in the variable
`BRIDGE_TOKEN` and stated in no file. `python tools/pin.py COMMIT` moves the pin: it writes
`bridge.txt` again from what the release of that commit holds.

**Where a change to Bridge is being tried, a folder is named instead**, which holds the two
packages as Bridge's build wrote them:

    python build.py --bridge DIR test

Either way each file is held against the hash its package states. What this project hands in
its turn states the commit of Bridge it was built against, so that a host can refuse a backend
built against another.

Bridge is another project. A document or a comment here cites one of Bridge's documents by its
name and a heading, and `bridge_headings.txt` is every heading of them at the commit this
backend was written against: `tools/bridge_headings.py` writes it again when a newer Bridge is
taken, and is the one tool that is handed another tree.

## What it hands

Two packages for a target, each a folder with its statement, `package.txt`: its name, this
project's commit, the commit of Bridge it was built against, the target, whether the cases ran
on it and passed, and the hash of each file.

    `backend_wasm`   the backend's library, wasmtime's shared library, which the backend loads
                     from beside itself, `license`, and wasmtime's own as `license_wasmtime`
    `sdk_wasm`       what a wasm module is written with: Bridge's `bridge_abi.h` and
                     `bridge_schema.h`, the files of `sdk/`, and `license`

A host takes the first, with Bridge's library. Whoever writes a wasm module takes the second.

## The tree

    src/                 the backend: the entries of Bridge's backend seam, over wasmtime
    sdk/                 what a wasm module is written with: `bridge_wasm.h`, which includes
                         the others, and the accessors of `bridge_wasm.wat` that are merged
                         into a compiled module
    wasmtime/            the engine it is linked against, as its authors published it
    utils/os/            the layer over the operating system, for each kind of platform
    tests/host/          a host of one backend, built from Bridge's package: the cases
    tests/adopt/         the backend called as the library calls it
    tests/support/       what the cases share
    tests/modules/       the modules the cases load
    tools/               what holds the code and these documents
    build.py             the build
    bridge.txt           the commit of Bridge this is built against, and its hashes

## The documents

    readme.md            this file
    wasm.md              how a wasm module is written: memories, accessors, exports
    wasmtime.md          the engine behind it: the pin, adoption, deadlines
    plan.md              the stages, each with a done-when that can fail
    contributing.md      how a decision is made, how a stage is run, where each rule is
    style.md             how the C is written: Bridge's coding style, made this project's
    tests.md             what is exposed, each with its cases
    dependencies.md      what a build needs installed, and what the backend imports
    licence.md           what it is under, and where other terms are had
    bridge_headings.txt  what a citation of Bridge's documents is checked against
    refused/             what was weighed and not chosen, under the same names

## Words

Bridge's words are Bridge's, and mean here what they mean there: host, backend, module, seam,
deck, phase, system, component, entity, record, package, work limit. One is this project's:

    SDK    the files a wasm module is written with: the imports, the accessors, and the
           layouts of Bridge's ABI read through them
