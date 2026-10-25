# BB-Wasm

BB-Wasm is the wasm backend of Bridge: it runs each module as sandboxed core wasm, over wasmtime.

## Background

Bridge loads modules onto a world, and a backend runs them. BB-Wasm is the backend `wasm`.
It compiles the bytes of a module into a wasm instance, and reaches each export by its name.
The backend does not trust its modules. It confines a module to the cells and the arena that it declared, and traps it if it reaches further.
So a module cannot read or write the memory of the world or of another module.

A module compiles against the SDK of this repository for wasm32: `docs/extern/bridge_wasm.h.md` tells how to write one.
The SDK includes `bridge_abi.h` and `bridge_schema.h`, thus a module also compiles with the headers of a release of Bridge.
The build of a module merges `acc.wasm`, the accessors of the SDK, into the module: `docs/intern/bridge_wasm.wat.md`, *The build*.

## Install

1. Install Python 3.11 or a later version.
2. Run `python project.py fetch`. It downloads zig, Bridge, Bridge-OS, wasmtime and binaryen.

## Usage

| Command | Result |
| --- | --- |
| `python project.py fetch` | Downloads what the build needs. |
| `python project.py build` | Builds the backend and `acc.wasm`. |
| `python project.py pack` | Packs the backend, the runtime of wasmtime, `acc.wasm` and the SDK for a release. |
| `python project.py test` | Builds the backend, and the programs and modules of the tests. Then runs the tests. |
| `python project.py test --break` | Runs the tests with their claims made false. Each test must fail. |
| `python project.py test --branches` | Runs the tests on a build with probes. Each branch of the code must run. |
| `python project.py test --leaks` | Runs the tests on a build with counts. Each test must give back what the code takes. |
| `python project.py check` | Checks the project. |
| `python project.py clean` | Removes what the build made. Keeps what `fetch` downloaded. |
| `python project.py distclean` | Removes the folder `build`, also what `fetch` downloaded. |

A host names the backend in its deployment, with the name `wasm` and the path of its library as `library`.
The library is `bridge_wasm.dll`, `bridge_wasm.so` or `bridge_wasm.dylib`. The runtime of wasmtime must be in the same folder.
The backend names the runtime by its file name only, so the loader finds it there on each machine.
The backend knows no setting.

## Messages

The backend writes one of these reasons when it refuses. Angle brackets mark a part that changes:

- `it knows no setting '<key>'`
- `its export '<name>' is not () -> i32`
- `no exported function '<name>'`

Bridge puts each reason into a record, after the name of the backend or of the module.

## Contributing

| Document | Content |
| --- | --- |
| `docs/contributing/coding_style.md` | How to write the code. |
| `docs/contributing/documentation_style.md` | How to write the documents. |
| `docs/contributing/tests.md` | How to write the tests. |
| `docs/contributing/commits.md` | How to write a commit. |
| `docs/contributing/pull_request.md` | How to write a pull request. |
| `docs/contributing/agreement.md` | The agreement `cla.md`, which each contributor signs. |

## License

The GNU Affero General Public License, version 3 only. The file `license` holds the text.
The release holds the runtime of wasmtime, whose notice is `third_party/wasmtime/license`.

The backend is built with the headers of Bridge, and Bridge loads it. Thus it is under the same licence, and its source goes with it.
A module that merges `acc.wasm` is a work based on BB-Wasm, and is under the same licence too.
For a work that stays closed, other terms are for sale: ask the account `Encrid` on the service that hosts this repository.
