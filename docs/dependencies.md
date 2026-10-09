# Dependencies

**Every thing BB-Wasm is built with or linked against is listed here, with why it is
there.** Third-party code is not refused. A dependency nobody wrote down is: `tools/deps.py` reads
this document and fails the build's tests on a tool `build.py` calls that is not listed, on a
function a built artifact imports that is not listed, and on one that is listed and that nothing
imports any more, so that the list is neither short nor long.

## What a build needs installed

A line is `tool`, the name `build.py` looks for, and why.

    tool      zig          the C compiler, the linker and the archiver, for every target
    tool      wasm-as      Binaryen: assembles the test modules and the SDK's accessors, which
                           are written as wasm text
    tool      wasm-merge   Binaryen: merges the SDK's accessors into a compiled wasm module
    tool      wasm-opt     Binaryen: puts the merged accessors inline - `wasm.md`, *Memories*
    tool      wasm-dis     Binaryen: prints a module, for the case that reads one

The four of Binaryen come in one archive, so what is installed is zig and Binaryen.

Python runs `build.py` and the scripts of `tools/`, with nothing outside its standard library.
That is also what fetches Bridge's two packages, at the commit `bridge.txt` states - `readme.md`,
*What it needs of Bridge*.

## What is in the tree

**wasmtime**, the engine the wasm backend is linked against: its C headers, its import library
and its shared library, for each target, with where each came from and its hash beside it -
`wasmtime.md`, *The pin*. The backend imports the functions of it that it calls, and nothing else
imports any:

    every     wasm_*       the wasm C API, as wasmtime implements it
    every     wasmtime_*   wasmtime's own C API: linear memories it does not allocate, fuel,
                           epochs, linkers

## What the built artifacts import

The backend, `bridge_wasm`. A line is the
system it is imported on, the function, and why. `every` is all three systems, and `posix` is
Linux and macOS; a system with a processor after it, `macos_x86_64`, is that one alone. A
function the compiler's own runtime provides is linked into the artifact and is not an import.

**The list is of what ships: the optimised build.** A debug build is linked with zig's own
checks and its stack traces, which import some sixty functions more on Windows, and none of that
is in a library a host is handed. The case says so when it is run on one, and holds nothing.

### The C library

    every     malloc       a backend, an instance, a module and what was found in one, each
    every     calloc       allocated when it is made
    every     free
    windows   strchr       whether a library's path has a directory in it
    windows   strncmp      imported with the startup code of the compiler: no source of ours
                           calls it
    posix     strlen       how long a name handed to wasmtime is. Inline on Windows
    linux     memcpy       a module's bytes and a signature copied. Inline on Windows
    macos     __memcpy_chk what the system's headers make of memcpy
    posix     snprintf     why a call was refused, written. On Windows it is a function of
                           the C runtime, below
    windows   __stdio_common_vsprintf
                           snprintf, as the C runtime has it

### The operating system, through `utils/os`

One layer, compiled whole into the backend - Bridge's `build.md`, *The C library*. It is
Bridge's, copied, and is kept as it is there: what opens a library and what answers which
thread this is are in it, are imported with it, and this backend calls neither.

    windows   VirtualAlloc              memory reserved and committed
    windows   VirtualFree               and released
    windows   LoadLibraryA              a library opened
    windows   LoadLibraryExA            with its own folder searched first
    windows   GetFullPathNameA          for which its path is made whole
    windows   GetProcAddress            a symbol of it found
    windows   FreeLibrary               and the library closed
    windows   CreateThread              the thread that keeps the deadlines started
    windows   WaitForSingleObject       a thread joined, and an event waited for
    windows   CloseHandle               a thread's or an event's handle closed
    windows   GetCurrentThreadId        which thread this is
    windows   QueryPerformanceCounter   the clock a deadline is kept by
    windows   QueryPerformanceFrequency and its unit
    windows   InitializeCriticalSection a mutex made
    windows   EnterCriticalSection      locked
    windows   LeaveCriticalSection      unlocked
    windows   DeleteCriticalSection     and freed
    windows   CreateEventA              an event made
    windows   SetEvent                  and set
    posix     mmap                      memory reserved
    posix     mprotect                  committed
    posix     munmap                    and released
    posix     dlopen                    a library opened
    posix     dlsym                     a symbol of it found
    posix     dlclose                   and the library closed
    posix     pthread_create            the thread that keeps the deadlines started
    posix     pthread_join              and joined
    posix     pthread_self              which thread this is
    posix     clock_gettime             the clock a deadline is kept by
    posix     pthread_mutex_init        a mutex made
    posix     pthread_mutex_lock        locked
    posix     pthread_mutex_unlock      unlocked
    posix     pthread_mutex_destroy     and freed
    posix     pthread_cond_init         an event made
    posix     pthread_cond_signal       set
    posix     pthread_cond_wait         waited for
    posix     pthread_cond_timedwait    waited for until a deadline
    posix     pthread_cond_destroy      and freed

### What the compiler's startup code imports

Nothing of ours calls these. zig links its startup code into every shared library it makes, and
on Windows that code imports them: it runs the library's initialisers, applies its relocations,
and says what went wrong if it cannot. They are here because they are imports all the same, and a
new one is a dependency that changed.

    windows   _initterm                 the initialisers run
    windows   _initterm_e
    windows   _initialize_onexit_table  the table of what runs at exit
    windows   _register_onexit_function
    windows   _execute_onexit_table
    windows   _exit                     the process ended, if the startup fails
    windows   abort
    windows   __acrt_iob_func           where it says so
    windows   __stdio_common_vfprintf
    windows   fwrite
    windows   VirtualProtect            the library's relocations applied
    windows   VirtualQuery
    windows   GetLastError
    windows   TlsGetValue               its thread-local storage
    windows   Sleep                     its start, waited for
    windows_aarch64  _fpreset           the floating-point state set, on arm64
    macos     dyld_stub_binder          the loader's own, which binds an import at its first call
