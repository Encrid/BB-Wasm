# Plan

The build order, and nothing else. What the backend is, is `wasm.md` and `wasmtime.md`; what
the documents are is `readme.md`.

**Every stage ends in a claim that can be false.** A stage is done when the stated thing is
observably true, and it is closed by the project's owner.

## 0. Out of Bridge

Not closed. It is the part of a stage of Bridge's plan that is done here, and is closed with
that one.

The backend, its SDK, the engine it wraps and the cases that prove them were written in
Bridge's tree. They are here as they were at Bridge's commit `ffda1a18ce70`, put where this
project keeps each, with the notice of each file made this project's. The cases are those
named for the SDK, every case of Bridge's host program that ran on this backend alone, and
those of the program that calls a backend as the library does, which has a loader of its own
here. And the method, the style and the tools are Bridge's, copied and made this project's.

*Done when the backend and its SDK are built from this tree and from Bridge's two packages
alone; every name the SDK exposes has a case, and every case passes; the code is held by the
style and the standard; and the documents cite nothing that is not there.*

## 1. Pinned and published

Not begun. It waits on Bridge publishing its two packages for a commit.

The build is handed a folder of Bridge's packages. This stage has it state one commit of
Bridge and the hashes of the two packages, fetch them from where Bridge publishes, and publish
in its turn, for each commit of `main` and each platform, the backend's package and the SDK's.

*Done when a build with no folder named fetches the two packages of the commit this tree
states and refuses one whose hash differs; the cases pass on each platform by a workflow; and
a run for `main` publishes the two packages of each platform, each stating the commit of
Bridge it was built against.*
