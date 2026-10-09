# The licence

What BB-Wasm is under, what that asks of what is built with it, whose it is, where other terms
are asked for, and what in the tree is somebody else's. This says what is meant, in plain
words. What binds is the text of `license`, and where the two differ, that text holds.

## What BB-Wasm is under

**The GNU Affero General Public License, version 3, and no other version.** Its text is
`license`, at the root, as its authors publish it. Nothing is added to it and nothing is
excepted from it. It is the licence Bridge is under.

**Everything in the tree that is ours is under it**: the backend, its SDK, the tests and their
modules, the tools and these documents. A source file says so in the line its header has for
it - `style.md`, *G1 - File header*.

`tools/licence.py` holds this, and is the case `licence`.

## What is built with it

**It is open too.** A deck opens the backend and runs in a host's process with it, so the host,
the library and the backend are one work. A module compiles the SDK into itself, has the
SDK's accessors merged into it, and is run by the backend, so the same holds for it. Whoever
passes such a work on, or lets others use it over a network, offers its source under this
licence or under one that may be combined with it.

Each file of the SDK says so where it opens, since that is where whoever writes a module looks.

That a work which links or compiles in another is one work with it is how licences of this
family are generally read; whether one that is only loaded is, has been argued both ways. This
says what is meant. Nobody who wrote this document is a lawyer.

## Other terms

**Whoever wants to keep closed what they build with BB-Wasm asks for other terms.** They are
had from the holder, and they are the only way a module or a host that runs on this backend may
be passed on without its source.

They are asked of the account `Encrid`, on the service this repository is hosted by.

## Whose it is

**The notices say "BB-Wasm Owner", and that is one person: its author, who is Bridge's.** The
owner is whoever holds the rights in it, which is its author until the author transfers them
in writing, and then whoever they were transferred to. The notices stay true through that.

No name of a person is in the tree.

**A contribution is taken with an agreement, `cla.md`.** A contributor keeps the copyright of
what they wrote and grants the holder a licence that may be passed on, under these terms and
under the others. It is signed by a sentence, as a comment on the pull request, and
`tools/cla.py` asks for it, records who signed on the branch `cla-signatures`, and fails a
pull request whose author has not.

## What is somebody else's

Each keeps its own licence.

    `wasmtime/`   wasmtime, as its authors published it: its C headers, an import library and
                  a shared library for each target. The Apache License, version 2.0, with the
                  LLVM exception; the text is in that folder. It may be combined with this
                  licence. Where each file came from, and its hash, is that folder's readme

`utils/os` and the tools came from Bridge, whose author is this project's: they were copied,
and are this project's here.

What a build needs installed, and what the built backend imports, is `dependencies.md`.
