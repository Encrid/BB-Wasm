/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "bridge_backend.h"

#ifdef _WIN32
    #include <windows.h>
    #define PROOF_LIBRARY "bridge_wasm.dll"
#else
    #include <dlfcn.h>
    #ifdef __APPLE__
        #define PROOF_LIBRARY "bridge_wasm.dylib"
    #else
        #define PROOF_LIBRARY "bridge_wasm.so"
    #endif
#endif

typedef uint32_t (*get_abi_fn_t)(void);

static size_t cut_two_folders(const char *exe)
{
    size_t n = strlen(exe);
    int seen = 0;

    while (n > 0) {
        n--;
        if ((exe[n] == '/' || exe[n] == '\\') && ++seen == 2)
            return (n + 1);
    }
    return (0);
}

static get_abi_fn_t find(char *path)
{
    get_abi_fn_t get = nullptr;
#ifdef _WIN32
    HMODULE lib;
    FARPROC found;

    for (char *at = path; *at; at++)
        *at = *at == '/' ? '\\' : *at;
    lib = LoadLibraryExA(path, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
    found = lib ? GetProcAddress(lib, "backend_get_abi") : nullptr;
#else
    void *lib = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    void *found = lib ? dlsym(lib, "backend_get_abi") : nullptr;
#endif

    if (found)
        memcpy(&get, &found, sizeof(get));
    return (get);
}

int main([[maybe_unused]] int argc, char **argv)
{
    char path[4096];
    size_t cut = cut_two_folders(argv[0]);
    get_abi_fn_t get;
    bool loads;

    if (cut + sizeof(PROOF_LIBRARY) > sizeof(path))
        return (1);
    memcpy(path, argv[0], cut);
    memcpy(path + cut, PROOF_LIBRARY, sizeof(PROOF_LIBRARY));
    get = find(path);
    loads = get && get() == BACKEND_ABI;
    printf("wasm package: %s\n", loads ? "the backend loads" : "no backend");
    return (loads ? 0 : 1);
}
