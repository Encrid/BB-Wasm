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

None is taken yet.

## The questions

    1  how the build is let into Bridge's release while Bridge is private: on a machine
       where the code is written, and in a workflow. A workflow needs a credential that
       reads another repository, and no file states one
    2  what `bridge.txt` states: the commit, a hash for which files, and what writes it
       when the pin moves
    3  what fetches: Python's standard library with the credential handed in, or a tool
       that `dependencies.md` would list
    4  what a workflow runs, and what it costs: every case once, six at once, on which of
       the six targets
    5  how a workflow gets Binaryen on Linux and macOS, at the version the build states

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
