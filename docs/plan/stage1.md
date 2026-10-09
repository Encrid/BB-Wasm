# Stage 1: pinned and published

Not built. `plan.md`, stage 1, is what it is for and when it is done. This document is the order
it is done in, what there was before it began, and each decision as it is taken. What a decision
refused is `stage1_refused.md`.

It follows from two rulings of the stage of Bridge's plan in which this project left its tree:
a backend takes the library by a pin, and from a folder where a change is being tried; and it
publishes its two packages, for each commit of its `main`, by the run that tests it.

## What there was (2026-10-09)

    the build          handed a folder with `--bridge DIR`, it holds each file of Bridge's two
                       packages against the hash the package states, and takes the one commit
                       both were made from. It writes this project's two packages for a
                       target, each stating that commit, and an archive of each
    where Bridge       a prerelease for each commit of its `main`, named `main-` and twelve
    publishes          characters of the commit: twelve archives, two packages for each of
                       six targets. Bridge is private, so an archive is fetched with a
                       credential and not by an address alone
    this repository    the branch `dev` alone. No `main`, no pin, and no workflow but the one
                       that asks for the agreement

## The order

    1  the stage        this document, on a branch `stage1` from `dev`
    2  the decisions    those under *The questions*, one at a time, before a file changes
    3  the pin          `bridge.txt`, and a build that fetches what it states when no folder
                        is named, by the same holding of hashes; a case that a file with
                        another hash is refused
    4  the workflows    one for each system, from Bridge's, made this project's
    5  `main`           created when Enkrid confirms it, and the first run for it read back
    6  close            put to Enkrid by the done-when

## The decisions

### How the build is let into Bridge's release

**In a workflow, by a GitHub App; where the code is written, by what that machine already
holds.** Enkrid ruled it on 2026-10-09, against what was recommended, which was a token of his
account that reads Bridge and nothing else.

An app is installed on Bridge, with leave to read its contents and no other. A workflow here
is handed the app's identifier and its key as two secrets of this repository, and has a token made
from them when a run begins: it reads Bridge, and it is good for about an hour. The build is
handed that token and does not know where it came from.

No file states the key, the identifier or a token. A workflow names the two secrets, and that is
all of them that is written.

What it costs: an app to make and to keep, two secrets where there was one, and a step in each
workflow that is somebody else's code, which makes the token. What it buys: nothing that reads
Bridge outlives a run, and nothing is tied to one person's account.

### What the pin states

**The commit, and the hash of each archive Bridge published for it.** Enkrid ruled it on
2026-10-09, as recommended. `bridge.txt`, at the root, is a line for the commit and twelve
after it: the SHA-256 of an archive and its name, for the two packages of each of the six
targets. It is the shape a package's own statement has.

**An archive is held against its line before it is opened.** What it holds is then taken as a
folder's is: each file against the hash the package states, and one commit for the two. So
nothing is unpacked that the pin does not name.

**A tool writes it**: `tools/pin.py`, handed a commit. It fetches what the release of that
commit holds, and writes the commit and the hashes. It believes what the release serves on the
day the pin moves, and no build believes it after.

What it costs: Bridge's run puts an archive in the place of one of the same name, so a run
repeated for one commit changes the bytes of each, by the times written in it, and the pin is
written again though nothing in a package differs. The build ends naming the archive.

### What fetches

**The build itself, with Python's standard library.** Enkrid ruled it on 2026-10-09, as
recommended. It asks the service for the release named for the pinned commit, and for each of
the two archives of the target by its number, with the token in a header.

**The token is read from one variable of the environment, `BRIDGE_TOKEN`**, on a machine and
in a run alike. With no folder named, nothing fetched and the variable not set, the build ends
saying both ways on: set it, or name `--bridge DIR`.

**An archive fetched and held stays under `build/`**, in a folder named for the commit, and is
not asked for again. So a token is needed once for a pin, and not for a build after it.

Nothing is installed for it: what a build needs is what it needed.

### What a workflow runs

**What Bridge's runs.** It follows from the ruling that this project is made from Bridge's
method and tools, and that it publishes by the run that tests it, and was not put.

One workflow a system. Linux and macOS on both processors; Windows on x86-64, where the target
for arm64 is compiled, linked and packaged from, its packages saying it was not run. Each job
fetches zig, has the token made, builds, runs every case once, writes the packages of its
target, runs each case six at once, and, for a push to `main`, attaches the archives to the
release named for the commit. A run starts for what reaches `main` and for a pull request, and
not for a change to documents alone.

What it costs is what it cost there, and is read off the first run.

### How a workflow gets Binaryen

**As Bridge's workflows did while the backend was in its tree.** It follows from the same
ruling, and was not put. On Linux and macOS the job fetches the archive of the version the
build states, from where its authors publish, and names the folder in `BINARYEN_ROOT`. On
Windows the build fetches it and holds it against its hash, as it does where the code is
written.

### How a run is started by hand

**A workflow can be started by hand, and one that runs for `main` publishes however it was
started.** Enkrid ruled it on 2026-10-09, as recommended. It was found when `main` was made: a
run starts for a push with a file in it that is not a document or a workflow, and a branch
made at a commit another branch already has is a push with no file in it, so nothing ran.
Bridge's `main` was there before its workflows were, and never met it.

A run started by hand on another branch tests and publishes nothing, as a pull request's does.

It stays of use after the first run: what `main` hands can be published again, for the same
commit, with no commit made for it.

## The questions

None stands. Five were listed when the stage was planned: three were put and ruled, and the
rest followed from a ruling that stands, each said as such under its heading.

## What is shown, and what is not

On 2026-10-09, of `main` at `3bd03458f302`, pinned to Bridge at `bb22256a976a`:

    the pin is taken        from a fresh clone, with no folder named: the two packages of the
                            commit `bridge.txt` states are fetched, each archive held, and
                            325 of 325 cases pass. With neither a token nor a folder, the
                            build ends saying both ways on
    another hash refused    the case `pin-another-archive-refused`
    the folder still works  `--bridge DIR`, 325 of 325
    each platform           the three runs for `main`, started by hand, read back: 325 of
                            325 once and six at once, on Linux and macOS on both processors
                            and on Windows on x86-64. Windows on arm64 is compiled, linked and
                            packaged, and not run. The token was made from the app in each job
    what it cost            75 seconds on Linux, 4 minutes on macOS and 7 minutes on Windows, from start to
                            end of each workflow
    what is published       the prerelease `main-3bd03458f302`: twelve archives, two packages for
                            each of six targets. The backend's statement, read back from each,
                            names this commit and Bridge's, and says whether it was run
    with the other two      a host built from the three projects' published archives for
                            Windows on x86-64, and from no build folder of any, loads a native
                            module and a wasm one and runs them three passes

    not shown               a run started by a push, there having been none with a file in
                            it since the workflows were written; and a pull request's run

## How each claim is shown

    the pin is taken        a fresh clone and a build with no folder named: the two packages
                            of the commit `bridge.txt` states are fetched, and every case
                            passes
    another hash refused    a case: a fetched archive with one byte changed, and the build
                            ends naming it
    the folder still works  `--bridge DIR` as before, every case
    each platform           the runs for `main`, read back: each job's cases
    what is published       an archive fetched back from the release: its statement names
                            this project's commit and Bridge's
