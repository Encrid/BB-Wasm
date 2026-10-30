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

### Settings

The deployment states each setting of the backend beside `library`, and no setting has a default.
The backend refuses a setting that is missing, and a key that it does not know.

| Key | Kind | Value |
| --- | --- | --- |
| `reservation` | integer | The bytes of address space of each memory. An own memory cannot start or grow past this size. |
| `guard` | integer | The bytes after each memory that have nothing mapped into them. |
| `max_wasm_stack` | integer | The most bytes of stack that the wasm frames of a call can use. A call that needs more traps. |
| `stack_margin` | integer | The bytes of stack, beyond `max_wasm_stack`, that each call keeps for the frames that are not wasm. |
| `watchdog_stack` | integer | The bytes of stack of the thread that stops a call at its timeout. |
| `opt_level` | text | How wasmtime optimizes the code of a module: `none`, `speed` or `speed_and_size`. |
| `parallel_compilation` | flag | Whether wasmtime compiles a module on threads of its own. |

An integer setting is a number of bytes, 0 or more, and `max_wasm_stack` is 1 or more.
With a guard of 0, wasmtime checks each access to a memory, since no page after the memory faults.
The stack that a module keeps in its own memory is a value of the build of the module, and no setting.

The frames that are not wasm come from the backend, and from a function that Bridge offers a module.
At each call of a module, the backend compares the stack that the thread has left with `max_wasm_stack` and `stack_margin` together.
If less is left, the backend refuses the call, so a deep call never overflows the stack of a thread: *Messages*.
Thus each thread that runs modules needs more stack than the two together.
That is each thread of the world, through `stack`, and the thread of the host.
The backend also refuses the call when the system does not tell the stack of the thread, and the reason then names 0 bytes.

### Limits

The backend makes the instance of a module at its first call, `init` or `reload`.
Thus the timeout of that call also bounds the start function, and an import that nobody answers fails that call.
The backend keeps each limit that Bridge sets:

- `work_limit` is the fuel of wasmtime, which Bridge sets before each call of a system.
- After each call, the fuel is full again. Thus `shutdown` never stops on the fuel that a system used up, and only `shutdown_timeout` bounds it.
- `memory_limit` bounds the bytes of the own memory of the module, and `element_limit` the elements of its own reference table.
- wasmtime bounds each memory and each table alone, and not their sum. Thus a module has one own memory and one own table.
- The memories that Bridge lends a module are not its own. A second own memory or a second own table fails the first call.

## Messages

The backend writes one of these reasons when it refuses a setting, an export or a call. Angle brackets mark a part that changes:

- `it knows no setting '<key>'`
- `<key> is unset`
- `<key> must be <kind>`, where `<kind>` is `a flag`, `an integer` or `a text`
- `<key> is <number> bytes, and it must be <least> or more`
- `opt_level is '<text>', and it can be none, speed or speed_and_size`
- `did not run: the thread has <number> bytes of stack left, and max_wasm_stack and stack_margin need <number>`
- `its export '<name>' is not () -> i32`
- `no exported function '<name>'`

Bridge puts each reason into a record, after the name of the backend or of the module.

## Contributing

| Document | Content |
| --- | --- |
| `docs/contributing/needs.md` | The needs that each decision meets. |
| `docs/contributing/decisions.md` | The steps of a decision. |
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
