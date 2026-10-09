"""BB-Wasm's build: one compiler, one script.

    python build.py --bridge DIR                 build, for this machine
    python build.py --bridge DIR test [-k NAME]  build, then run the cases (all, or those matching)
    python build.py --bridge DIR test --copies 6 each case six times at once: under load
    python build.py --bridge DIR --target T      one of TARGETS, below
    python build.py --bridge DIR --debug         unoptimised, with debug information
    python build.py --bridge DIR package         build, run every case, and make what is handed

Needs zig on the path. Binaryen is taken from BINARYEN_ROOT or the path, and on Windows fetched
at its pin when it is in neither. DIR is a folder that holds the two packages of Bridge,
`bridge` and `seams`, each with its statement: they are taken as they are stated, and a file
that is not what its hash says is refused. Every path is taken from where this file is and
from --out.
"""
import argparse
import concurrent.futures
import hashlib
import os
import platform
import re
import shutil
import subprocess
import sys
import tarfile
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent
# What differs by target, and nothing else does: where it runs, how a program and a library are
# named, which folder of utils/os is its operating system's, which of the system's declarations are asked for by name (the language is ISO C, and a
# compiler's gnu mode would have shown them unasked), what a link names, how a library finds what
# sits beside it, and wasmtime's files under wasmtime: the one that backend is linked
# against, and the one it loads.
TARGETS = {
    'x86_64-windows-gnu': {
        'runs on': ('Windows', 'AMD64'), 'program': '.exe', 'library': '.dll', 'os': 'windows',
        'compile': [], 'link': [], 'beside': [], 'named': [],
        'wasmtime': ('windows/libwasmtime.dll.a', 'windows/wasmtime.dll')},
    'x86_64-linux-gnu': {
        'runs on': ('Linux', 'x86_64'), 'program': '', 'library': '.so', 'os': 'posix',
        'compile': ['-fPIC', '-D_DEFAULT_SOURCE'], 'link': ['-lm', '-lpthread', '-ldl'],
        'beside': ['-Wl,-rpath,$ORIGIN'], 'named': ['-Wl,-soname,bridge.so'],
        'wasmtime': ('linux/libwasmtime.so', 'linux/libwasmtime.so')},
    'aarch64-macos': {
        'runs on': ('Darwin', 'arm64'), 'program': '', 'library': '.dylib', 'os': 'posix',
        'compile': ['-fPIC', '-D_DARWIN_C_SOURCE'], 'link': [],
        'beside': ['-Wl,-rpath,@loader_path'], 'named': ['-Wl,-install_name,@rpath/bridge.dylib'],
        'wasmtime': ('macos/libwasmtime.dylib', 'macos/libwasmtime.dylib')},
    'aarch64-linux-gnu': {
        'runs on': ('Linux', 'aarch64'), 'program': '', 'library': '.so', 'os': 'posix',
        'compile': ['-fPIC', '-D_DEFAULT_SOURCE'], 'link': ['-lm', '-lpthread', '-ldl'],
        'beside': ['-Wl,-rpath,$ORIGIN'], 'named': ['-Wl,-soname,bridge.so'],
        'wasmtime': ('linux_aarch64/libwasmtime.so', 'linux_aarch64/libwasmtime.so')},
    'x86_64-macos': {
        'runs on': ('Darwin', 'x86_64'), 'program': '', 'library': '.dylib', 'os': 'posix',
        'compile': ['-fPIC', '-D_DARWIN_C_SOURCE'], 'link': [],
        'beside': ['-Wl,-rpath,@loader_path'], 'named': ['-Wl,-install_name,@rpath/bridge.dylib'],
        'wasmtime': ('macos_x86_64/libwasmtime.dylib', 'macos_x86_64/libwasmtime.dylib')},
    'aarch64-windows-gnu': {
        'runs on': ('Windows', 'ARM64'), 'program': '.exe', 'library': '.dll', 'os': 'windows',
        'compile': [], 'link': [], 'beside': [], 'named': [],
        'wasmtime': ('windows_aarch64/wasmtime.dll.lib', 'windows_aarch64/wasmtime.dll')},
}
BINARYEN = 'binaryen-version_133'
BINARYEN_URL = ('https://github.com/WebAssembly/binaryen/releases/download/version_133/'
                'binaryen-version_133-x86_64-windows.tar.gz')
BINARYEN_SHA256 = '17a2cbeac6b5693c5fbafab3838d3c65fd9c1eb38b05f5baec6c657e8c84995b'
FEATURES = ['--enable-multimemory', '--enable-bulk-memory', '--enable-sign-ext',
            '--enable-nontrapping-float-to-int', '--enable-mutable-globals']
TIMEOUT = 120

OS = ['memory', 'library', 'thread', 'mutex', 'event']
WASM_BACKEND = ['wasm_fail', 'wasm_own', 'wasm_adopted', 'wasm_holder', 'wasm_engine', 'wasm_backend',
        'wasm_module', 'wasm_instance', 'wasm_call', 'wasm_offered', 'wasm_func']

# a module in C: the name, its source, whether its rows are tested, what it is compiled with.
# `checked` or `unchecked` is stated for each, as the SDK asks: BRIDGE_ROWS_CHECKED, or not.
PLAIN = ('mover reader refs fp badterm spin3 flood chatty rows halver twostep watcher chain '
         'badobs events fixer nested namer spawner counter typer user hang hit '
         'shield aim levels tagged')
WASM = [(name, name, 'checked', []) for name in PLAIN.split()] + [
    ('rows-unchecked', 'rows', 'unchecked', []),
    ('nested2', 'nested', 'checked', ['LEAVE_OUT_VEC2']),
    ('settings', 'settings', 'checked', ['KEPT=BRIDGE_SINGLETON']),
    ('kept-unset', 'kept', 'checked', ['KEPT=0']), ('kept-none', 'kept', 'checked', ['KEPT=9']),
    ('kept-sparse', 'kept', 'checked', ['KEPT=BRIDGE_SPARSE_SET']),
    ('namer-reader', 'namer', 'checked', ['READER']),
    ('flaky-always', 'flaky', 'checked', ['OF=1', 'FAILS=1']),
    ('flaky-third', 'flaky', 'checked', ['OF=3', 'FAILS=1']),
    ('flaky-twothirds', 'flaky', 'checked', ['OF=3', 'FAILS=2']),
    ('flaky-calm', 'flaky', 'checked', ['OF=1', 'FAILS=1', 'CALM']),
    ('keeper-strip', 'keeper', 'checked', ['PRIVATE_POLICY=BRIDGE_STRIP']),
    ('keeper-sweep', 'keeper', 'checked', ['PRIVATE_POLICY=BRIDGE_SWEEP']),
    ('keeper-delete', 'keeper', 'checked', ['PRIVATE_POLICY=BRIDGE_DELETE']),
    ('retype-private', 'retype', 'checked', ['WHICH=1']),
    ('retype-vec2', 'retype', 'checked', ['WHICH=2']),
    ('retype-extra', 'retype', 'checked', ['WHICH=3']),
    ('retype-settings', 'retype', 'checked', ['WHICH=4']),
    ('settings-drop', 'settings', 'checked', ['DROP', 'KEPT=BRIDGE_SINGLETON']),
    ('settings-table', 'settings', 'checked', ['KEPT=BRIDGE_TABLE']),
    ('ticker', 'ticker', 'checked', ['STEP=1']),
    ('ticker-ten', 'ticker', 'checked', ['STEP=10']),
    ('ticker-wide', 'ticker', 'checked', ['STEP=10', 'WIDE']),
    ('ticker-fail', 'ticker', 'checked', ['STEP=10', 'FAIL_RELOAD']),
    ('ticker-noextra', 'ticker', 'checked', ['STEP=10', 'NO_EXTRA']),
    ('ticker-trip', 'ticker', 'checked', ['STEP=1', 'TRIP']),
    ('ticker-alloc', 'ticker', 'checked', ['STEP=10', 'ALLOC']),
]
# the wrong module: each build does one thing an entry refuses, at init or in its system, and
# states that it does nothing in the other
WRONG_AT_INIT = ('declare_outside declare_name_outside declare_no_namespace declare_kind_unset '
                 'declare_tag_with_bytes declare_policy_unset declare_size_other system_outside '
                 'system_phase_outside system_name_outside system_many_changes system_twice '
                 'observer_no_term observe_outside observe_events_unset term_access_unset '
                 'term_access_other term_oper_unset term_slot_undeclared term_write_ungranted '
                 'change_slot_undeclared change_ungranted alloc_stated_none')
WRONG_IN_SYSTEM = ('declare_late system_late observe_late attach_undeclared attach_ungranted '
                   'attach_outside detach_undeclared attach_tag_value')
WASM += [('wrong-' + act.replace('_', '-'), 'wrong', 'checked',
          ['AT_INIT=wrong_' + act, 'IN_SYSTEM=wrong_do_nothing']) for act in WRONG_AT_INIT.split()]
WASM += [('wrong-' + act.replace('_', '-'), 'wrong', 'checked',
          ['AT_INIT=wrong_do_nothing', 'IN_SYSTEM=wrong_' + act]) for act in WRONG_IN_SYSTEM.split()]


def die(msg):
    sys.stderr.write('build: %s\n' % msg)
    sys.exit(1)


def stale(outs, deps):
    if not all(o.exists() for o in outs):
        return True
    oldest = min(o.stat().st_mtime for o in outs)
    return any(d.stat().st_mtime > oldest for d in deps)


def step(what, outs, deps, commands):
    """Run `commands` if an output is missing or older than a dependency."""
    if not stale(outs, deps):
        return
    for o in outs:
        o.parent.mkdir(parents=True, exist_ok=True)
    for command in commands:
        if callable(command):
            command()
            continue
        done = subprocess.run([str(c) for c in command], stdout=subprocess.PIPE,
                              stderr=subprocess.STDOUT)
        if done.returncode:
            for o in outs:
                if o.exists():
                    o.unlink()
            # how it ended is said with what it said: a process the system ended says nothing
            raise RuntimeError('%s\n  %s\n  ended with %d (0x%08X)\n%s'
                               % (what, ' '.join(str(c) for c in command), done.returncode,
                                  done.returncode & 0xFFFFFFFF,
                                  done.stdout.decode('utf-8', 'replace')))
        if done.stdout.strip():
            sys.stdout.write(done.stdout.decode('utf-8', 'replace'))


def together(steps):
    """Steps that do not depend on each other, at once."""
    with concurrent.futures.ThreadPoolExecutor(os.cpu_count()) as pool:
        failed = [f.exception() for f in [pool.submit(step, *s) for s in steps] if f.exception()]
    if failed:
        die('\n'.join(str(f) for f in failed))


def tool(name, hints=()):
    exe = name + ('.exe' if os.name == 'nt' else '')
    for hint in hints:
        if (Path(hint) / exe).exists():
            return Path(hint) / exe
    found = shutil.which(name)
    return Path(found) if found else None


def binaryen(build):
    """The four tools of Binaryen a build calls: from BINARYEN_ROOT, the path, or fetched at the
    pin. wasm-as assembles what is written as wasm text, wasm-merge and wasm-opt put the SDK's
    accessors into a compiled module, wasm-dis prints one for the case that reads it."""
    hints = [build / BINARYEN / 'bin']
    if os.environ.get('BINARYEN_ROOT'):
        hints.insert(0, Path(os.environ['BINARYEN_ROOT']) / 'bin')
    if not tool('wasm-merge', hints):
        if os.name != 'nt':
            die('wasm-merge is not on the path, and BINARYEN_ROOT does not name it')
        archive = build / 'binaryen.tar.gz'
        build.mkdir(parents=True, exist_ok=True)
        urllib.request.urlretrieve(BINARYEN_URL, archive)
        if hashlib.sha256(archive.read_bytes()).hexdigest() != BINARYEN_SHA256:
            die('%s is not the Binaryen this build pins: its hash differs' % archive)
        with tarfile.open(archive) as tar:
            tar.extractall(build)
    found = [tool('wasm-as', hints), tool('wasm-merge', hints), tool('wasm-opt', hints),
             tool('wasm-dis', hints)]
    if not all(found):
        die('Binaryen was found without wasm-as, wasm-merge, wasm-opt or wasm-dis')
    return found


def take(folder):
    """The two packages of Bridge, from a folder that holds them: each file held against the
    hash its package states, and the commit of Bridge both were made from, which is one."""
    if not folder:
        die('name the folder of Bridge\'s packages: --bridge DIR')
    commits = set()
    for name in ('bridge', 'seams'):
        stated = folder / name / 'package.txt'
        if not stated.is_file():
            die('%s is not there: %s does not hold the package %s' % (stated, folder, name))
        for line in stated.read_text(encoding='utf-8').splitlines():
            head, _, rest = line.partition(' ')
            if head == 'commit':
                commits.add(rest)
            if len(head) == 64 and rest.strip():
                held = folder / name / rest.strip()
                if not held.is_file() or hashlib.sha256(held.read_bytes()).hexdigest() != head:
                    die('%s is not what the package %s states' % (held, name))
    if len(commits) != 1:
        die('the two packages of Bridge are of different commits: %s' % ', '.join(sorted(commits)))
    return commits.pop()


# What is handed. A package is a folder, and this is what each holds: a file of the tree, of
# the build or of Bridge's seams, and its name in the package where that differs.
def packages(out, lib, bridge, runtime):
    licence = (ROOT / 'license', 'license')
    return {
        'backend_wasm': [(out / 'backends' / ('bridge_wasm' + lib), 'bridge_wasm' + lib),
                         (out / 'backends' / runtime.name, runtime.name), licence,
                         (ROOT / 'wasmtime/LICENSE', 'license_wasmtime')],
        'sdk_wasm': [(bridge / 'seams' / h, h) for h in ('bridge_abi.h', 'bridge_schema.h')] +
                    [(f, f.name) for f in sorted((ROOT / 'sdk').iterdir())] + [licence],
    }


def commit():
    """The commit the tree is at, and whether the tree is that commit and nothing more."""
    def git(*words):
        return subprocess.run(['git'] + list(words), cwd=ROOT, capture_output=True, text=True).stdout.strip()
    changed = git('status', '--porcelain', '--untracked-files=no')
    return git('rev-parse', 'HEAD') + (' with changes that are not committed' if changed else '')


def write_packages(target, out, lib, bridge, runtime, ran):
    """Each package of a target, as a folder under packages/, with its statement: its name, the
    commit, the commit of Bridge it was built against, the target, whether the cases were run
    and passed on it, and the SHA-256 of each file."""
    at, against = commit(), take(bridge)
    for name, files in packages(out, lib, bridge, runtime).items():
        folder = out / 'packages' / name
        folder.mkdir(parents=True, exist_ok=True)
        said = ['package %s' % name, 'commit %s' % at, 'bridge %s' % against, 'target %s' % target,
                'run %s' % ('yes' if ran else 'no')]
        for source, called in files:
            data = source.read_bytes()
            if not (folder / called).exists() or (folder / called).read_bytes() != data:
                (folder / called).write_bytes(data)
            said.append('%s  %s' % (hashlib.sha256(data).hexdigest(), called))
        for stray in set(p.name for p in folder.iterdir()) - {c for _, c in files} - {'package.txt'}:
            (folder / stray).unlink()
        text = '\n'.join(said) + '\n'
        if not (folder / 'package.txt').exists() or (folder / 'package.txt').read_text() != text:
            (folder / 'package.txt').write_text(text, newline='\n')


def archive_packages(target, out):
    """Each package as one archive, named for the package and the target: what is published."""
    made = []
    for folder in sorted(p for p in (out / 'packages').iterdir() if p.is_dir()):
        path = out / 'packages' / ('%s-%s.tar.gz' % (folder.name, target))
        with tarfile.open(path, 'w:gz') as tar:
            for f in sorted(folder.iterdir()):
                tar.add(f, arcname='%s/%s' % (folder.name, f.name))
        made.append(path)
    return made


def build(target, debug, out, bridge):
    row = TARGETS[target]
    exe, lib = row['program'], row['library']
    zig = tool('zig') or die('zig is not on the path')
    take(bridge)
    assemble, merge, opt, disassemble = binaryen(out.parent)
    obj, modules, backends = out / 'obj', out / 'modules', out / 'backends'
    wasmtime = ROOT / 'wasmtime'
    link_wasmtime, runtime = (wasmtime / f for f in row['wasmtime'])
    cc = [zig, 'cc', '-target', target] + (['-O0', '-g'] if debug else ['-O2', '-DNDEBUG'])
    cc += row['compile']
    ours = cc + ['-std=c23', '-pedantic-errors', '-Wall', '-Wextra', '-Werror',
                 '-Wno-missing-field-initializers']
    system, here = row['link'], row['beside']
    # what is Bridge's is its two packages, and no path of its tree
    host_seam, include = '-I%s' % (bridge / 'bridge'), '-I%s' % (bridge / 'seams')
    os_dir = ROOT / 'utils/os' / row['os']
    utils = '-I%s' % (ROOT / 'utils/os')
    sdk = {'wasm': '-I%s' % (ROOT / 'sdk')}
    stated = [bridge / name / 'package.txt' for name in ('bridge', 'seams')]
    headers = [Path(__file__)] + stated + [h for d in ('src', 'sdk', 'tests/*', 'tests/modules',
                                                       'tests/modules/*', 'utils/os',
                                                       'utils/os/' + row['os'])
                                           for h in ROOT.glob(d + '/*.h')]
    defines = ['-D%s="%s"' % (key, path.as_posix()) for key, path in
               [('BACKEND_PATH', backends / ('bridge_wasm' + lib)), ('MODULE_DIR', modules)]]
    compiles = []

    def compile_(name, source, flags):
        compiles.append(('compiling %s' % name, [obj / (name + '.o')], [source] + headers,
                         [flags + ['-c', source, '-o', obj / (name + '.o')]]))
        return obj / (name + '.o')

    # the backend: the backend seam, the operating system's layer, the engine's headers, and
    # what is its own
    backend_o = [compile_(name, ROOT / 'src' / (name + '.c'),
                          ours + [include, utils, '-I%s' % (wasmtime / 'include')])
                 for name in WASM_BACKEND] + \
                [compile_('os-' + name, os_dir / (name + '.c'), ours + [utils]) for name in OS]
    # a host of the tests is compiled against bridge.h and linked with the library, as a host is
    support = '-I%s' % (ROOT / 'tests/support')
    host_o = [compile_('host-' + c.stem, c, ours + [host_seam, include, support] + defines)
              for c in sorted((ROOT / 'tests/host').glob('*.c')) +
              sorted((ROOT / 'tests/support').glob('host*.c')) +
              sorted((ROOT / 'tests/support' / row['os']).glob('host*.c'))]
    together(compiles)

    links = []

    def link(name, objects, flags=(), shared=False):
        target_file = out / (name + (lib if shared else exe))
        links.append(('linking %s' % name, [target_file], list(objects),
                      [cc + (['-shared'] if shared else []) + list(objects) + list(flags) +
                       ['-o', target_file]]))

    # the backend loads wasmtime from beside itself, which is not beside the programs: a host is
    # not where its backends are
    step('copying wasmtime', [backends / runtime.name], [runtime],
         [lambda: shutil.copyfile(runtime, backends / runtime.name)])
    link('backends/bridge_wasm', backend_o, [link_wasmtime] + system + here, shared=True)
    beside = out / ('bridge' + lib)
    step('putting the library beside the host', [beside], [bridge / 'bridge' / beside.name],
         [lambda: shutil.copyfile(bridge / 'bridge' / beside.name, beside)])
    link('host', host_o + [bridge / 'bridge' / beside.name], system + here)
    together(links)
    shared = '-I%s' % (ROOT / 'tests/modules')

    # ---- wasm modules: text assembled, C compiled and merged with the accessors ----
    accessors = ROOT / 'sdk/bridge_wasm.wat'
    acc = modules / 'acc.wasm'
    assembled = [('assembling %s' % wat.name, [modules / (wat.stem + '.wasm')], [wat],
                  [[assemble, wat] + FEATURES + ['-o', modules / (wat.stem + '.wasm')]])
                 for wat in sorted((ROOT / 'tests/modules').glob('*.wat'))]
    assembled += [('assembling the accessors', [acc], [accessors],
                   [[assemble, accessors] + FEATURES + ['-o', acc]])]
    together(assembled)
    compiled = []
    for name, source, kind, flags in WASM:
        raw, merged, final = (modules / (name + s) for s in ('.raw.wasm', '.merged.wasm', '.wasm'))
        c = sources(ROOT / 'tests/modules', source, 'idle')
        compiled.append(('building module %s' % name, [final], c + [acc] + headers, [
            [zig, 'cc', '-target', 'wasm32-freestanding', '-std=c23', '-pedantic-errors', '-Wall',
             '-Wextra', '-Werror', '-Wno-missing-field-initializers', '-O2', '-nostdlib'] +
            ['-D' + d for d in flags + ['BRIDGE_ROWS_' + kind.upper()]] +
            [include, sdk['wasm'], shared, '-Wl,--no-entry', '-o', raw] + c,
            [merge, raw, 'main', acc, 'acc'] + FEATURES + ['-o', merged],
            [opt, merged] + FEATURES + ['-O3', '--remove-exports=acc_*',
                                         '--remove-unused-module-elements', '-o', final]]))
    together(compiled)

    return {'python': sys.executable, 'nocall': ROOT / 'tools/nocall.py',
            'wasm-dis': disassemble, 'modules': modules, 'exe': exe}


def sources(where, name, idle):
    """A module's sources: the one file of its name, or every C file of the directory of its
    name - docs/style.md, O3, allows five functions a file, and a module may export more. A
    module that includes the idle header of its backend states that its shutdown and its reload
    do nothing, and is built with the source that has them."""
    one = where / (name + '.c')
    found = [one] if one.is_file() else sorted((where / name).glob('*.c'))
    stated = any('_idle.h"' in c.read_text(encoding='utf-8') for c in found)
    return found + ([where / (idle + '.c')] if stated else [])


def command_line(out, built, command, kept):
    """What a case runs: a program of the build, or the tool that reads a compiled module."""
    if command[0] == 'nocall':
        return [str(built[k]) for k in ('python', 'nocall', 'wasm-dis')] + FEATURES + \
               [str(built['modules'] / command[1]), command[2]]
    return [str(out / (command[0] + built['exe']))] + command[1:]


def run(argv, out):
    """One copy of a case: what it said, and how it ended."""
    try:
        done = subprocess.run(argv, cwd=out, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                              timeout=TIMEOUT)
        return done.stdout.decode('utf-8', 'replace'), done.returncode
    except subprocess.TimeoutExpired as late:
        return (late.stdout or b'').decode('utf-8', 'replace'), 'timeout'


def judge(said, code, match, never):
    """Why a copy of a case failed, or None. With a pattern, the pattern decides and the exit
    code does not: several cases are a refusal that ends the program. Without one, the exit
    code does."""
    if code == 'timeout':
        return 'did not end in %d seconds' % TIMEOUT
    if never and re.search(never, said, re.S):
        return 'said what it must not: %s' % never
    if match and match.startswith('@') and \
            said.replace('\r', '') != (ROOT / match[1:]).read_text(encoding='utf-8'):
        return 'did not say what %s holds, word for word' % match[1:]
    if match and not match.startswith('@') and not re.search(match, said, re.S):
        return 'ended with %s and did not say: %s' % (code, match)
    if not match and code:
        return 'ended with %s' % code
    return None


def test(out, built, only, copies):
    """Each case by what it said. With copies, each case is started that many times at once,
    and every copy is judged as a case is: a case is a process, and the copies compete for the
    processors, for memory, and for what a process shares with its neighbours."""
    cases = [c for c in CASES if not only or only in c[0]]
    failed = []
    # what every case said is kept, a file a case: tools/said.py holds each message of the
    # source against it. A run of every case begins with none, so that no case that is gone
    # holds a message
    kept = out / 'said'
    if not only:
        shutil.rmtree(kept, ignore_errors=True)
    kept.mkdir(exist_ok=True)
    for name, command, match, never in cases:
        argv = command_line(out, built, command, kept)
        count = 1 if name in ALONE else copies
        with concurrent.futures.ThreadPoolExecutor(count) as pool:
            ran = list(pool.map(lambda _: run(argv, out), range(count)))
        (kept / (name + '.txt')).write_text(ran[0][0], encoding='utf-8')
        wrong = [(said, why) for said, why in ((s, judge(s, c, match, never)) for s, c in ran) if why]
        if not wrong:
            print('ok    %s' % name)
            continue
        print('FAIL  %s: %s, in %d of %d\n%s' % (name, wrong[0][1], len(wrong), count, wrong[0][0]))
        failed.append(name)
    print('%d of %d passed' % (len(cases) - len(failed), len(cases)))
    if failed:
        die('failed: %s' % ' '.join(failed))
    if not cases:
        die('no test is named like %r' % only)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('command', nargs='?', default='build', choices=['build', 'test', 'package'])
    ap.add_argument('--bridge', type=Path, metavar='DIR', help='the folder of Bridge\'s two packages')
    ap.add_argument('--target', choices=sorted(TARGETS))
    ap.add_argument('--debug', action='store_true')
    ap.add_argument('--out', type=Path, help='where to build: build/<target> beside this file')
    ap.add_argument('-k', metavar='NAME', help='only the cases whose name has this in it')
    ap.add_argument('--copies', type=int, default=1, metavar='N',
                    help='start each case N times at once, and judge every copy')
    ap.add_argument('--rounds', type=int, default=1, metavar='N', help='run the cases N times')
    args = ap.parse_args()
    machine = (platform.system(), platform.machine())
    mine = next((t for t, row in TARGETS.items() if row['runs on'] == machine), None)
    target = args.target or mine or die('no target runs on %s %s: name one' % machine)
    if args.command == 'test' and target != mine:
        die('%s cannot be run here: build it, and test it where it runs' % target)
    bridge = args.bridge.resolve() if args.bridge else None
    out = (args.out or ROOT / 'build' / (target + ('-debug' if args.debug else ''))).resolve()
    built = build(target, args.debug, out, bridge)
    if args.command == 'package':
        if target == mine:
            test(out, built, None, 1)
        row = TARGETS[target]
        write_packages(target, out, row['library'], bridge, ROOT / 'wasmtime' / row['wasmtime'][1],
                       target == mine)
        for path in archive_packages(target, out):
            print(path)
    if args.command == 'test':
        for _ in range(args.rounds):
            test(out, built, args.k, args.copies)


# Every case: its name, what is run, what its output must match, what it must not. A case ends
# in "alive": checked by what was said, and by the process still being there to say it.
CASES = [
    ('bridge_load-loaded', ['host', 'bridge_load-loaded'],
     'test:ok@0.1.0: loaded.*alive', None),
    ('BRIDGE_NO_LIMIT-stated-for-every-limit', ['host', 'BRIDGE_NO_LIMIT-stated-for-every-limit'],
     'test:ok@0.1.0: loaded.*alive', None),
    ('bridge_load-memory-with-a-data-segment', ['host', 'bridge_load-memory-with-a-data-segment'],
     'test:image@0.1.0: loaded.*alive', None),
    ('bridge_load-init-trapped', ['host', 'bridge_load-init-trapped'],
     "module 'test:oob@0.1.0' did not load: its init trapped: out of bounds memory access.*alive", None),
    ('module_config.init_timeout-init-interrupted', ['host', 'module_config.init_timeout-init-interrupted'],
     "module 'test:spin@0.1.0' did not load: its init trapped: interrupt.*alive", None),
    ('module_config.memory_limit-growth-refused', ['host', 'module_config.memory_limit-growth-refused'],
     "module 'test:grow@0.1.0' did not load: its init answered 7.*alive", None),
    ('module_config.memory_limit-memory-larger-at-start', ['host', 'module_config.memory_limit-memory-larger-at-start'],
     "module 'test:big@0.1.0' did not load: cannot instantiate.*alive", None),
    ('instance_get_func-export-taking-an-argument-refused', ['host', 'instance_get_func-export-taking-an-argument-refused'],
     "module 'test:shape@0.1.0' did not load: its export 'init' is not .. -> i32.*alive", None),
    ('instance_get_func-export-answering-64-bits-refused', ['host', 'instance_get_func-export-answering-64-bits-refused'],
     "module 'test:shape64@0.1.0' did not load: its export 'init' is not .. -> i32.*alive", None),
    ('bridge_load-no-init-export', ['host', 'bridge_load-no-init-export'],
     "module 'test:noinit@0.1.0' did not load: no exported function 'init'.*alive", None),
    ('module_config.abi-another-refused', ['host', 'module_config.abi-another-refused'],
     "module 'test:ok@0.1.0' is refused: it wants ABI 2 and this is ABI 1.*alive", None),
    ('artifact.backend-not-configured-refused', ['host', 'artifact.backend-not-configured-refused'],
     "module 'test:ok@0.1.0' is refused: no backend named 'native'.*alive", None),
    ('module_config.log_level-unset-refused', ['host', 'module_config.log_level-unset-refused'],
     "module 'test:ok@0.1.0' is refused: log_level is unset.*alive", ': loaded'),
    ('module_config.memory_limit-unset-refused', ['host', 'module_config.memory_limit-unset-refused'],
     "module 'test:ok@0.1.0' is refused: memory_limit is unset.*alive", None),
    ('BRIDGE_NONE-for-a-limit-refused', ['host', 'BRIDGE_NONE-for-a-limit-refused'],
     "module 'test:ok@0.1.0' is refused: spawns is -1, which is neither an amount nor BRIDGE_NONE.*module 'test:ok@0.1.0' is refused: work_limit is -2, which is neither a limit nor BRIDGE_NO_LIMIT.*alive", ': loaded'),
    ('config.abi-another-refused', ['host', 'config.abi-another-refused'],
     'the host is of host ABI 2, and this is 1.*bridge_create refused.*alive', None),
    ('config.abi-unset-refused', ['host', 'config.abi-unset-refused'],
     'abi is unset.*bridge_create refused.*alive', None),
    ('BRIDGE_UNSET-flag-refused', ['host', 'BRIDGE_UNSET-flag-refused'],
     "module 'test:ok@0.1.0' is refused: pinned is unset.*module 'test:ok@0.1.0' is refused: trusted is 3, which is neither BRIDGE_TRUE nor BRIDGE_FALSE.*alive", ': loaded'),
    ('module_config.memory_limit-negative-refused', ['host', 'module_config.memory_limit-negative-refused'],
     "module 'test:ok@0.1.0' is refused: memory_limit is -7, which is neither a limit nor BRIDGE_NO_LIMIT.*alive", ': loaded'),
    ('module_config.on_error-fail-stops-and-skip-does-not', ['host', 'module_config.on_error-fail-stops-and-skip-does-not'],
     "test:ok@0.1.0: loaded.*the deck has stopped: module 'test:oob@0.1.0' failed and its on_error is fail.*test:ok@0.1.0: not loaded.*alive", None),
    ('backend.library-cannot-open-refused', ['host', 'backend.library-cannot-open-refused'],
     "backend 'wasm': cannot open backend library 'no-such-backend-library'.*bridge_create refused.*alive", None),
    ('bridge_create-no-sink-refused', ['host', 'bridge_create-no-sink-refused'],
     'refused: no sink.*alive', None),
    ('BRIDGE_ROW-no-call-in-the-loop', ['nocall', 'mover.wasm', 'move'],
     None, None),
    ('module_config.read-another-namespace-read', ['host', 'module_config.read-another-namespace-read'],
     'sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive', None),
    ('bridge_get_span-one-table-walked', ['host', 'bridge_get_span-one-table-walked'],
     'view:reader@0.1.0/sum: saw 1 500.*game:mover@0.1.0/count: moved 2 1000.*alive', None),
    ('record.level-info-with-its-two-numbers', ['host', 'record.level-info-with-its-two-numbers'],
     'record.6. view:reader@0.1.0/sum: sum 250500 0.*alive', None),
    ('bridge_get_ref-own-entities-followed', ['host', 'bridge_get_ref-own-entities-followed'],
     'mirror 100 0.*mirror 100 0.*mirror 100 0.*alive', None),
    ('declare-storage-unset-refused', ['host', 'declare-storage-unset-refused'],
     "module 'game:kept@0.1.0' did not load: type 'game:Position': its storage layout is unset.*module 'game:kept@0.1.0' did not load: type 'game:Position' is stated as a sparse set here, and it was declared as a table. It is also declared by 'game:mover@0.1.0'.*alive", None),
    ('BRIDGE_DIRECT_INDEXED-reached-from-rows', ['host', 'BRIDGE_DIRECT_INDEXED-reached-from-rows'],
     'game:aim@0.1.0/reached: aims 10 5.*game:aim@0.1.0/reached: aimed 20 0.*alive', 'did not load'),
    ('BRIDGE_TOGGLE-counted-on-and-off', ['host', 'BRIDGE_TOGGLE-counted-on-and-off'],
     'game:shield@0.1.0/count: shields 10 10.*game:shield@0.1.0/count: strength 45 0.*game:shield@0.1.0/count: shields 10 6.*game:shield@0.1.0/count: strength 27 0.*alive', 'did not load'),
    ('BRIDGE_SPARSE_SET-walked-and-reached', ['host', 'BRIDGE_SPARSE_SET-walked-and-reached'],
     'game:hit@0.1.0/walked: hits 4 18.*game:hit@0.1.0/reached: bodies 10 4.*game:hit@0.1.0/reached: reached 18 0.*alive', 'did not load'),
    ('declare-another-layout-refused', ['host', 'declare-another-layout-refused'],
     "module 'game:fp@0.1.0' did not load: type 'game:Position' has another component layout here: fingerprint [0-9a-f]+, and it was declared with [0-9a-f]+. It is also declared by 'game:mover@0.1.0'.*alive", None),
    ('declare-ungranted-namespace-refused', ['host', 'declare-ungranted-namespace-refused'],
     "module 'view:reader@0.1.0' did not load: it may not declare 'game:Position': that is another namespace's, and no grant names it.*alive", None),
    ('add_system-unknown-operator-refused', ['host', 'add_system-unknown-operator-refused'],
     "module 'game:badterm@0.1.0' did not load: term 0 of system 'any': its operator is 4, and there are three.*alive", None),
    ('bridge_lend-scratch-given-back', ['host', 'bridge_lend-scratch-given-back'],
     'game:mover@0.1.0: loaded.*moved.*alive', 'not loaded'),
    ('module_config.work_limit-spinning-system-ended', ['host', 'module_config.work_limit-spinning-system-ended'],
     "test:spin@0.1.0/forever: system 'forever' failed: trapped: all fuel consumed.*still here.*all fuel consumed.*still here.*alive", None),
    ('module_config.spawns-used-up', ['host', 'module_config.spawns-used-up'],
     "system 'flood' failed: spawn: its spawns of 3000 between two barriers are used up.*system 'flood' failed: spawn: .budget. entities of 4096 is reached.*alive", None),
    ('module_config.log_ring-full-lines-dropped', ['host', 'module_config.log_ring-full-lines-dropped'],
     'line 15 0.*test:chatty@0.1.0/-: 84 log lines were dropped: its ring holds 16.*alive', None),
    ('module_config.spans-too-small-refused', ['host', 'module_config.spans-too-small-refused'],
     "module 'view:reader@0.1.0' did not load: its spans of 16 bytes do not hold a column_base of 2 components by \\d+ tables.*moved 2 1000.*alive", None),
    ('budget.columns-used-up-changes-not-applied', ['host', 'budget.columns-used-up-changes-not-applied'],
     "module 'game:mover@0.1.0': .budget. columns of 4096 bytes is used up. Not applied, of the changes it asked: 1244 .*moved 1 256.*alive", 'has stopped'),
    ('bridge_run_phase-undeclared-phase-refused', ['host', 'bridge_run_phase-undeclared-phase-refused'],
     "phase 'render' is not one the deployment declared.*alive", None),
    ('config.phases-none-refused', ['host', 'config.phases-none-refused'],
     'the deployment declares no phase.*bridge_create refused.*alive', None),
    ('budget.tables-unset-refused', ['host', 'budget.tables-unset-refused'],
     '.budget. tables is unset.*bridge_create refused.*alive', None),
    ('budget.types-unset-refused', ['host', 'budget.types-unset-refused'],
     '.budget. types is unset.*bridge_create refused.*alive', None),
    ('budget.systems-unset-refused', ['host', 'budget.systems-unset-refused'],
     '.budget. systems is unset.*bridge_create refused.*alive', None),
    ('budget.types-reached-refused', ['host', 'budget.types-reached-refused'],
     "module 'game:mover@0.1.0' did not load: type 'game:Marked' is one more than .budget. types of 1 .*not loaded.*alive", None),
    ('budget.systems-reached-refused', ['host', 'budget.systems-reached-refused'],
     "module 'game:mover@0.1.0' did not load: it registers one more than .budget. systems of 1 .*not loaded.*alive", None),
    ('module_config.log_level-no-severity-refused', ['host', 'module_config.log_level-no-severity-refused'],
     "module 'game:mover@0.1.0' is refused: its log_level of 8 is not one it can be, which is 1 to 7.*not loaded.*alive", None),
    ('module_config.log_level-warnings-drop-info', ['host', 'module_config.log_level-warnings-drop-info'],
     'game:mover@0.1.0: loaded\\s+alive', 'moved'),
    ('budget.terms-unset-refused', ['host', 'budget.terms-unset-refused'],
     '.budget. terms is unset.*bridge_create refused.*alive', None),
    ('budget.terms-exceeded-refused', ['host', 'budget.terms-exceeded-refused'],
     "module 'view:reader@0.1.0' did not load: a system asks for 2 terms, which is more than .budget. terms of 1 .*not loaded.*alive", None),
    ('budget.columns-unset-refused', ['host', 'budget.columns-unset-refused'],
     '.budget. columns is unset.*bridge_create refused.*alive', None),
    ('budget.entities-beyond-32-bits-refused', ['host', 'budget.entities-beyond-32-bits-refused'],
     '.budget. entities of 1099511627776 is more than an offset of 32 bits can reach.*bridge_create refused.*alive', None),
    ('bridge_create-budget-sum-beyond-32-bits-refused', ['host', 'bridge_create-budget-sum-beyond-32-bits-refused'],
     '.budget. cannot be satisfied: the shared memory would be [0-9]+ bytes and an offset into it is 32 bits.*bridge_create refused.*alive', None),
    ('module_config.scratch-private-beyond-32-bits-refused', ['host', 'module_config.scratch-private-beyond-32-bits-refused'],
     "module 'game:mover@0.1.0' is refused: its scratch of 4294967040, log_ring of [0-9]+ and spans of 4096 bytes come to more than an offset of 32 bits can reach.*alive", None),
    ('BRIDGE_ROW-checked-and-unchecked-timed', ['host', 'BRIDGE_ROW-checked-and-unchecked-timed'],
     'rows-unchecked  no limit  [0-9.]+ ns a row.*alive', None),
    ('config.order-mover-before-halver', ['host', 'config.order-mover-before-halver'],
     'sum 125250 0.*alive', None),
    ('config.order-halver-before-mover', ['host', 'config.order-halver-before-mover'],
     'sum 125500 0.*alive', None),
    ('order-unordered-conflict-refused', ['host', 'order-unordered-conflict-refused'],
     "module 'view:halver@0.1.0' did not load: 'game:mover:move' and 'view:halver:halve' both touch 'game:Position' in phase 'update', one of them writing it, and nothing orders them.*alive", None),
    ('add_system-own-conflict-in-registration-order', ['host', 'add_system-own-conflict-in-registration-order'],
     'game:twostep@0.1.0/show: x 3 0.*alive', None),
    ('config.order-cycle-refused', ['host', 'config.order-cycle-refused'],
     '.order. contradicts itself.*bridge_create refused.*alive', None),
    ('observe-chain-one-hop-a-barrier', ['host', 'observe-chain-one-hop-a-barrier'],
     'b1 game:chain@0.1.0/got_a: hop A.*b2 game:chain@0.1.0/got_b: hop B.*b3 game:chain@0.1.0/got_c: hop C.*alive', 'hop A.*hop A'),
    ('observe-reserved-name-refused', ['host', 'observe-reserved-name-refused'],
     "'init' is a reserved name, and no observer may take it.*alive", None),
    ('config.threads-unset-refused', ['host', 'config.threads-unset-refused'],
     'threads is unset.*bridge_create refused.*alive', None),
    ('config.threads-four-wasm-instances-hop', ['host', 'config.threads-four-wasm-instances-hop'],
     'sum 275000 0.*alive', None),
    ('bridge_get_trigger_events-enter-attach-leave', ['host', 'bridge_get_trigger_events-enter-attach-leave'],
     'any: event 3 1.*any: event 2 2.*any: event 4 57005.*alive', None),
    ('declare-nested-types', ['host', 'declare-nested-types'],
     'read: body 2 2.*read: more 3 4.*alive', None),
    ('declare-undeclared-field-type-refused', ['host', 'declare-undeclared-field-type-refused'],
     "module 'game:nested@0.1.0' did not load: type 'game:Body': its descriptor says 'vec2_t', which is neither a type a field can be nor one its namespace has declared.*alive", None),
    ('bridge_create-two-decks-at-once', ['host', 'bridge_create-two-decks-at-once'],
     'two decks: sums 275000 and 265000.*alive', None),
    ('alloc-string-kept', ['host', 'alloc-string-kept'],
     'game:namer@0.1.0/say: kept in the arena.*alive', None),
    ('alloc-share-exhausted', ['host', 'alloc-share-exhausted'],
     "module 'game:namer@0.1.0' did not load: alloc: its persistent of 4 bytes is exhausted.*alive", None),
    ('module_config.persistent-more-than-the-arena-refused', ['host', 'module_config.persistent-more-than-the-arena-refused'],
     "module 'game:namer@0.1.0' is refused: its persistent of 1048576 bytes does not fit in the 4096 left of .budget. persistent.*alive", None),
    ('spawn-stated-as-none-refused', ['host', 'spawn-stated-as-none-refused'],
     "module 'game:mover@0.1.0' did not load: spawn: it stated that it spawns nothing.*alive", None),
    ('bridge_load-failed-init-undone', ['host', 'bridge_load-failed-init-undone'],
     'game:badterm@0.1.0: not loaded.*game:fp@0.1.0: loaded.*alive', None),
    ('spawn-same-ids-on-one-thread-and-four', ['host', 'spawn-same-ids-on-one-thread-and-four'],
     'minted 160 ids a run. on four threads the same, and again the same.*alive', None),
    ('BRIDGE_SKIP-failed-init-skipped', ['host', 'BRIDGE_SKIP-failed-init-skipped'],
     'game:badobs@0.1.0: not loaded.*show: x 3 0.*modules: game:twostep@0.1.0=RUNNING .1..*alive', None),
    ('bridge_load-refused-leaves-nothing', ['host', 'bridge_load-refused-leaves-nothing'],
     'game:badterm@0.1.0: not loaded.*game:fp@0.1.0: loaded.*alive', None),
    ('module_config.max_system_failures-reached-stops-the-module', ['host', 'module_config.max_system_failures-reached-stops-the-module'],
     "module 'test:flaky@0.1.0' is stopped: its system 'trip' reached its max_system_failures of 3.*show: x 63 0.*game:twostep@0.1.0=RUNNING test:flaky@0.1.0=STOPPED.*alive", None),
    ('module_config.max_system_failures-one-in-three-forgiven', ['host', 'module_config.max_system_failures-one-in-three-forgiven'],
     'modules: test:flaky@0.1.0=RUNNING.*alive', None),
    ('module_config.max_system_failures-two-in-three-reached', ['host', 'module_config.max_system_failures-two-in-three-reached'],
     "its system 'trip' reached its max_system_failures of 3.*modules: test:flaky@0.1.0=STOPPED.*alive", None),
    ('module_config.max_system_failures-clean-system-forgives-no-other', ['host', 'module_config.max_system_failures-clean-system-forgives-no-other'],
     "its system 'trip' reached its max_system_failures of 3.*modules: test:flaky@0.1.0=STOPPED.*alive", None),
    ('module_config.max_system_failures-no-limit-never-stops', ['host', 'module_config.max_system_failures-no-limit-never-stops'],
     'modules: test:flaky@0.1.0=RUNNING.*alive', None),
    ('BRIDGE_FAIL-stopped-module-stops-the-deck', ['host', 'BRIDGE_FAIL-stopped-module-stops-the-deck'],
     "the deck has stopped: module 'test:flaky@0.1.0' was stopped and its on_error is fail.*update refused.*test:flaky@0.1.0=STOPPED.*game:twostep@0.1.0: not unloaded.*alive", None),
    ('bridge_unload-of-what-stopped-the-deck', ['host', 'bridge_unload-of-what-stopped-the-deck'],
     "the deck has stopped.*game:twostep@0.1.0: not unloaded.*the deck runs again: module 'test:flaky@0.1.0' is unloaded, and it goes on without it.*test:flaky@0.1.0: unloaded.*modules: game:twostep@0.1.0=RUNNING.*alive", None),
    ('BRIDGE_STRIP-unload-strips', ['host', 'BRIDGE_STRIP-unload-strips'],
     'left 10 5.*game:keeper@0.1.0: unloaded.*left 9 5.*game:retype@0.1.0: loaded.*alive', None),
    ('BRIDGE_SWEEP-unload-sweeps', ['host', 'BRIDGE_SWEEP-unload-sweeps'],
     'left 10 5.*game:keeper@0.1.0: unloaded.*left 9 0.*game:retype@0.1.0: loaded.*alive', None),
    ('BRIDGE_DELETE-unload-deletes', ['host', 'BRIDGE_DELETE-unload-deletes'],
     'left 10 5.*game:keeper@0.1.0: unloaded.*left 0 0.*game:retype@0.1.0: loaded.*alive', None),
    ('bridge_unload-declarer-goes-user-holds-the-type', ['host', 'bridge_unload-declarer-goes-user-holds-the-type'],
     'game:typer@0.1.0: unloaded.*read: body 2 3.*game:retype@0.1.0: not loaded.*game:user@0.1.0: unloaded.*game:retype@0.1.0: loaded.*alive', None),
    ('bridge_unload-not-loaded-refused', ['host', 'bridge_unload-not-loaded-refused'],
     "no module 'game:nobody@0.1.0' is loaded, so none is unloaded.*alive", None),
    ('module_config.shutdown_timeout-shutdown-interrupted', ['host', 'module_config.shutdown_timeout-shutdown-interrupted'],
     'its shutdown trapped: interrupt. its teardown continues.*test:hang@0.1.0: unloaded.*modules: .0..*alive', None),
    ('budget.modules-reached-refused', ['host', 'budget.modules-reached-refused'],
     "module 'game:ticker@0.1.0' is refused: .budget. modules of 1 is reached.*game:twostep@0.1.0: unloaded.*game:ticker@0.1.0: loaded.*modules: game:ticker@0.1.0=RUNNING .1..*alive", None),
    ('bridge_reload-larger-share', ['host', 'bridge_reload-larger-share'],
     'game:ticker@0.2.0: reloaded.*tick 112 10.*tick 122 10.*alive', None),
    ('bridge_reload-larger-share-past-a-neighbour', ['host', 'bridge_reload-larger-share-past-a-neighbour'],
     'game:namer@0.1.0: loaded.*game:ticker@0.2.0: reloaded.*tick 112 10.*alive', None),
    ('bridge_reload-same-share-exhausted', ['host', 'bridge_reload-same-share-exhausted'],
     'was not reloaded: alloc: its persistent of 16 bytes is exhausted.*tick 103 1.*tick 104 1.*alive', None),
    ('bridge_reload-smaller-share-refused', ['host', 'bridge_reload-smaller-share-refused'],
     "is not reloaded: its persistent is 16 and a reload states less, which a loaded module's share cannot become.*tick 104 1.*alive", None),
    ('bridge_reload-share-more-than-the-arena-refused', ['host', 'bridge_reload-share-more-than-the-arena-refused'],
     'is not reloaded: its persistent of 1048576 bytes does not fit in the [0-9]+ left of .budget. persistent.*tick 104 1.*alive', None),
    ('bridge_reload-lifts-a-stopped-deck', ['host', 'bridge_reload-lifts-a-stopped-deck'],
     "the deck has stopped: module 'game:ticker@0.1.0' was stopped and its on_error is fail.*game:twostep@0.1.0: not loaded.*the deck runs again: module 'game:ticker@0.2.0' is back.*game:ticker@0.2.0: reloaded.*tick 1[0-9]3 10.*game:ticker@0.2.0=RUNNING.*alive", None),
    ('bridge_load-lifts-a-stopped-deck', ['host', 'bridge_load-lifts-a-stopped-deck'],
     "the deck has stopped: module 'game:thing@0.1.0' failed and its on_error is fail.*game:other@0.1.0: not loaded.*the deck runs again: module 'game:thing@0.2.0' is back.*game:thing@0.2.0: loaded.*show: x 3 0.*alive", None),
    ('bridge_reload-stopped-module-revived', ['host', 'bridge_reload-stopped-module-revived'],
     'game:ticker@0.1.0=STOPPED.*game:ticker@0.2.0: reloaded.*tick 1[0-9]3 10.*game:ticker@0.2.0=RUNNING.*alive', None),
    ('bridge_load-no-reload-export', ['host', 'bridge_load-no-reload-export'],
     "module 'test:noreload@0.1.0' did not load: no exported function 'reload'.*alive", None),
    ('module_config.reloadable-false-with-hot-reload-refused', ['host', 'module_config.reloadable-false-with-hot-reload-refused'],
     "module 'game:twostep@0.1.0' is refused: hot_reload is stated, and its author states it is not reloadable.*alive", None),
    ('bridge_reload-not-loaded-refused', ['host', 'bridge_reload-not-loaded-refused'],
     'is not reloaded: nothing of that name is loaded.*alive', None),
    ('module_config.settings-of-every-kind', ['host', 'module_config.settings-of-every-kind'],
     'vsync width 1 1280.*scale mode 150 2.*show: hello 7 0.*show: some/long/path.cfg 0 0.*flags ratio 3 3.*show: big 2 0.*alive', None),
    ('setting.key-no-such-field-refused', ['host', 'setting.key-no-such-field-refused'],
     "its settings have a key 'vsynk', and 'game:Settings' has no field of that name.*alive", None),
    ('setting.kind-text-for-a-float-refused', ['host', 'setting.kind-text-for-a-float-refused'],
     "its setting 'scale' is text, and the field it names is a float.*alive", None),
    ('BRIDGE_STRING-longer-than-the-field-refused', ['host', 'BRIDGE_STRING-longer-than-the-field-refused'],
     "its setting 'title' is 20 bytes of text, and the field it names holds 16 with the zero that ends it.*alive", None),
    ('BRIDGE_INT-beyond-the-field-refused', ['host', 'BRIDGE_INT-beyond-the-field-refused'],
     "its setting 'small' is 300, which the field it names cannot hold.*alive", None),
    ('BRIDGE_INT-beyond-the-enum-refused', ['host', 'BRIDGE_INT-beyond-the-enum-refused'],
     "its setting 'mode' is 200, which the enum of the field it names cannot hold.*alive", None),
    ('BRIDGE_STRING-no-such-constant-refused', ['host', 'BRIDGE_STRING-no-such-constant-refused'],
     "its setting 'mode' is 'Sometimes', which the enum of the field it names has no constant for.*alive", None),
    ('BRIDGE_NO_CONFIG-with-settings-refused', ['host', 'BRIDGE_NO_CONFIG-with-settings-refused'],
     'it is handed 10 settings, and states that it has no config component.*alive', None),
    ('module_config.config-unset-refused', ['host', 'module_config.config-unset-refused'],
     "module 'game:settings@0.1.0' is refused: config is unset.*alive", None),
    ('module_config.config-undeclared-refused', ['host', 'module_config.config-undeclared-refused'],
     "its config is 'game:Nope', which is not a struct its init declared.*alive", None),
    ('module_config.settings-text-beyond-the-share-refused', ['host', 'module_config.settings-text-beyond-the-share-refused'],
     'its settings hold 24 bytes of text, and its persistent share has 8 left.*alive', None),
    ('module_config.config-taken-refused', ['host', 'module_config.config-taken-refused'],
     "its config is 'game:Settings', which is already the settings of module 'game:settings@0.1.0'.*alive", None),
    ('bridge_reload-settings-row-kept', ['host', 'bridge_reload-settings-row-kept'],
     'vsync width 1 1280.*game:settings@0.2.0: reloaded.*vsync width 1 1280.*alive', None),
    ('bridge_reload-config-dropped', ['host', 'bridge_reload-config-dropped'],
     'game:settings@0.2.0: reloaded.*game:retype@0.1.0: loaded.*alive', None),
    ('module_config.settings-reseeded-in-place', ['host', 'module_config.settings-reseeded-in-place'],
     'settings text written to the same place.*alive', None),
    ('budget.columns-no-room-for-a-singleton-refused', ['host', 'budget.columns-no-room-for-a-singleton-refused'],
     "did not load: type 'game:Settings' is a singleton, and .budget. columns of 16 bytes has no room for its \\d+ bytes.*alive", None),
    ('module_config.config-not-a-singleton-refused', ['host', 'module_config.config-not-a-singleton-refused'],
     "did not load: its config is 'game:Settings', which is not declared a singleton.*alive", None),
    ('artifact.bytes-none-for-wasm', ['host', 'artifact.bytes-none-for-wasm'],
     "module 'view:reader@0.1.0' did not load: this backend compiles a module from its bytes, and was handed none.*alive", None),
    ('bridge_unload-two-hundred-rounds', ['host', 'bridge_unload-two-hundred-rounds'],
     'churned 200 times, 0 loads refused, 0 records of the index out of step.*alive', None),
    ('bridge_unload-share-given-back', ['host', 'bridge_unload-share-given-back'],
     'game:namer@0.1.0/say: kept in the arena.*game:namer@0.1.0: unloaded.*game:ticker@0.1.0: loaded.*alive', None),
    ('bridge_unload-share-kept-while-pointed-into', ['host', 'bridge_unload-share-kept-while-pointed-into'],
     "module 'game:namer@0.1.0' is unloaded and its share of the persistent arena, 64 bytes, stays: 'game:Name' holds a string in it, in this many rows: 1.*game:namer@0.1.0: unloaded.*game:reader@0.1.0/say: kept in the arena.*module 'game:ticker@0.1.0' is refused: its persistent of 64 bytes does not fit in the 0 left of .budget. persistent.*game:reader@0.1.0/say: kept in the arena.*alive", None),
    ('bridge_destroy-shuts-down-what-is-loaded', ['host', 'bridge_destroy-shuts-down-what-is-loaded'],
     "tick 101 1.*game:ticker@0.1.0/-: shutdown 1 0.*module 'test:hang@0.1.0': its shutdown trapped: interrupt. its teardown continues.*alive", None),
    ('budget.entities-unset-refused', ['host', 'budget.entities-unset-refused'],
     '.budget. entities is unset.*alive', None),
    ('budget.modules-unset-refused', ['host', 'budget.modules-unset-refused'],
     '.budget. modules is unset.*alive', None),
    ('budget.persistent-unset-refused', ['host', 'budget.persistent-unset-refused'],
     '.budget. persistent is unset.*alive', None),
    ('budget.sparse_sets-unset-refused', ['host', 'budget.sparse_sets-unset-refused'],
     '.budget. sparse_sets is unset.*alive', None),
    ('budget.sparse_sets-negative-refused', ['host', 'budget.sparse_sets-negative-refused'],
     '.budget. sparse_sets is -5, which is neither a count nor none.*alive', None),
    ('budget.sparse_sets-beyond-32-bits-refused', ['host', 'budget.sparse_sets-beyond-32-bits-refused'],
     '.budget. cannot be satisfied: sparse_sets of 16777216, with four bytes for each of 4096 entities, is more than an offset of 32 bits can reach.*alive', None),
    ('budget.direct_indexed-unset-refused', ['host', 'budget.direct_indexed-unset-refused'],
     '.budget. direct_indexed is unset.*alive', None),
    ('budget.direct_indexed-no-limit-refused', ['host', 'budget.direct_indexed-no-limit-refused'],
     '.budget. direct_indexed is -1, which is neither a count nor none.*alive', None),
    ('budget.direct_indexed-beyond-32-bits-refused', ['host', 'budget.direct_indexed-beyond-32-bits-refused'],
     '.budget. cannot be satisfied: direct_indexed of 16777216, with a bit for each of 4096 entities, is more than an offset of 32 bits can reach.*alive', None),
    ('config.phases-nameless-refused', ['host', 'config.phases-nameless-refused'],
     'phase 1 has no name.*alive', None),
    ('config.phases-twice-refused', ['host', 'config.phases-twice-refused'],
     "phase 'update' is named twice.*alive", None),
    ('order.before-none-refused', ['host', 'order.before-none-refused'],
     '.order. pair 0 does not name two systems.*alive', None),
    ('order.after-none-refused', ['host', 'order.after-none-refused'],
     '.order. pair 0 does not name two systems.*alive', None),
    ('module_config.identity-none-refused', ['host', 'module_config.identity-none-refused'],
     'a module with no identity is refused.*alive', None),
    ('module_config.trusted-unset-refused', ['host', 'module_config.trusted-unset-refused'],
     "module 'test:ok@0\\.1\\.0' is refused: trusted is unset.*alive", None),
    ('module_config.reloadable-unset-refused', ['host', 'module_config.reloadable-unset-refused'],
     "module 'test:ok@0\\.1\\.0' is refused: reloadable is unset.*alive", None),
    ('module_config.hot_reload-unset-refused', ['host', 'module_config.hot_reload-unset-refused'],
     "module 'test:ok@0\\.1\\.0' is refused: hot_reload is unset.*alive", None),
    ('module_config.on_error-unset-refused', ['host', 'module_config.on_error-unset-refused'],
     "module 'test:ok@0\\.1\\.0' is refused: on_error is unset.*alive", None),
    ('module_config.max_system_failures-unset-refused', ['host', 'module_config.max_system_failures-unset-refused'],
     "module 'test:ok@0\\.1\\.0' is refused: max_system_failures is unset.*alive", None),
    ('module_config.work_limit-unset-refused', ['host', 'module_config.work_limit-unset-refused'],
     "module 'test:ok@0\\.1\\.0' is refused: work_limit is unset.*alive", None),
    ('module_config.init_timeout-unset-refused', ['host', 'module_config.init_timeout-unset-refused'],
     "module 'test:ok@0\\.1\\.0' is refused: init_timeout is unset.*alive", None),
    ('module_config.reload_timeout-unset-refused', ['host', 'module_config.reload_timeout-unset-refused'],
     "module 'test:ok@0\\.1\\.0' is refused: reload_timeout is unset.*alive", None),
    ('module_config.shutdown_timeout-unset-refused', ['host', 'module_config.shutdown_timeout-unset-refused'],
     "module 'test:ok@0\\.1\\.0' is refused: shutdown_timeout is unset.*alive", None),
    ('module_config.scratch-unset-refused', ['host', 'module_config.scratch-unset-refused'],
     "module 'test:ok@0\\.1\\.0' is refused: scratch is unset.*alive", None),
    ('module_config.log_ring-unset-refused', ['host', 'module_config.log_ring-unset-refused'],
     "module 'test:ok@0\\.1\\.0' is refused: log_ring is unset.*alive", None),
    ('module_config.log_ring-smaller-than-a-record-refused', ['host', 'module_config.log_ring-smaller-than-a-record-refused'],
     "module 'test:ok@0\\.1\\.0' is refused: a log_ring of 8 bytes holds no record, and one is 24.*alive", None),
    ('module_config.spans-unset-refused', ['host', 'module_config.spans-unset-refused'],
     "module 'test:ok@0\\.1\\.0' is refused: spans is unset.*alive", None),
    ('module_config.spawns-unset-refused', ['host', 'module_config.spawns-unset-refused'],
     "module 'test:ok@0\\.1\\.0' is refused: spawns is unset.*alive", None),
    ('module_config.spawns-more-than-entities-refused', ['host', 'module_config.spawns-more-than-entities-refused'],
     "module 'test:ok@0\\.1\\.0' is refused: its spawns of 5000 is more than .budget. entities of 4096.*alive", None),
    ('module_config.persistent-unset-refused', ['host', 'module_config.persistent-unset-refused'],
     "module 'test:ok@0\\.1\\.0' is refused: persistent is unset.*alive", None),
    ('module_config.changes-unset-refused', ['host', 'module_config.changes-unset-refused'],
     "module 'test:ok@0\\.1\\.0' is refused: changes is unset.*alive", None),
    ('module_config.changes-beyond-32-bits-refused', ['host', 'module_config.changes-beyond-32-bits-refused'],
     "module 'test:ok@0\\.1\\.0' is refused: its changes of 8589934592 bytes is more than an offset of 32 bits can reach.*alive", None),
    ('module_config.losses-count-unset-refused', ['host', 'module_config.losses-count-unset-refused'],
     "module 'test:ok@0\\.1\\.0' is refused: loss_count is unset.*alive", None),
    ('module_config.losses-counted-and-none-refused', ['host', 'module_config.losses-counted-and-none-refused'],
     "module 'test:ok@0\\.1\\.0' is refused: it states 2 losses, and losses is unset.*alive", None),
    ('bridge_load-loaded-already-refused', ['host', 'bridge_load-loaded-already-refused'],
     "module 'test:ok@0\\.1\\.0' is refused: 'test:ok@0\\.1\\.0' is loaded already.*alive", None),
    ('module_config.spawns-sum-beyond-entities-refused', ['host', 'module_config.spawns-sum-beyond-entities-refused'],
     "module 'test:image@0\\.1\\.0' did not load: its spawns of 3000 do not fit under .budget. entities of 4096.*alive", None),
    ('phase_name.from-unset-refused', ['host', 'phase_name.from-unset-refused'],
     "module 'test:ok@0\\.1\\.0' is refused: phase name 1 of its config is unset.*alive", None),
    ('module_config.phases-mapped-twice-refused', ['host', 'module_config.phases-mapped-twice-refused'],
     "module 'test:ok@0\\.1\\.0' is refused: its config maps phase 'tick' twice.*alive", None),
    ('declare-statement-outside-scratch-refused', ['host', 'declare-statement-outside-scratch-refused'],
     "module 'game:wrong@0\\.1\\.0' did not load: declare: what it states lies outside the scratch arena.*alive", None),
    ('bridge_declaration_t-name-outside-scratch-refused', ['host', 'bridge_declaration_t-name-outside-scratch-refused'],
     "module 'game:wrong@0\\.1\\.0' did not load: declare: a name or a descriptor lies outside the scratch arena.*alive", None),
    ('declare-name-with-no-namespace-refused', ['host', 'declare-name-with-no-namespace-refused'],
     "module 'game:wrong@0\\.1\\.0' did not load: type 'Odd' has no namespace.*alive", None),
    ('declare-kind-unset-refused', ['host', 'declare-kind-unset-refused'],
     "module 'game:wrong@0\\.1\\.0' did not load: type 'game:Odd': its kind is unset.*alive", None),
    ('BRIDGE_TAG-with-bytes-refused', ['host', 'BRIDGE_TAG-with-bytes-refused'],
     "module 'game:wrong@0\\.1\\.0' did not load: type 'game:Odd' is a tag, and a tag has no bytes and no descriptor.*alive", None),
    ('declare-on-unload-unset-refused', ['host', 'declare-on-unload-unset-refused'],
     "module 'game:wrong@0\\.1\\.0' did not load: type 'game:Odd': on_unload is unset.*alive", None),
    ('declare-size-other-than-descriptor-refused', ['host', 'declare-size-other-than-descriptor-refused'],
     "module 'game:wrong@0\\.1\\.0' did not load: type 'game:Odd' is 12 bytes aligned to 4 in the module and 8 aligned to 4 by its descriptor.*alive", None),
    ('add_system-statement-outside-scratch-refused', ['host', 'add_system-statement-outside-scratch-refused'],
     "module 'game:wrong@0\\.1\\.0' did not load: add_system: what it states lies outside the scratch arena.*alive", None),
    ('bridge_system_statement_t-phase-outside-scratch-refused', ['host', 'bridge_system_statement_t-phase-outside-scratch-refused'],
     "module 'game:wrong@0\\.1\\.0' did not load: add_system: a phase lies outside the scratch arena.*alive", None),
    ('bridge_system_statement_t-name-outside-scratch-refused', ['host', 'bridge_system_statement_t-name-outside-scratch-refused'],
     "module 'game:wrong@0\\.1\\.0' did not load: system: a name, a term or a change lies outside the scratch arena.*alive", None),
    ('add_system-more-changes-than-types-refused', ['host', 'add_system-more-changes-than-types-refused'],
     "module 'game:wrong@0\\.1\\.0' did not load: a system states 5 changes, and the module declared 3 types.*alive", None),
    ('add_system-name-twice-refused', ['host', 'add_system-name-twice-refused'],
     "module 'game:wrong@0\\.1\\.0' did not load: 'act' is registered twice.*alive", None),
    ('observe-no-term-refused', ['host', 'observe-no-term-refused'],
     "module 'game:wrong@0\\.1\\.0' did not load: observer 'again' has no term, and so nothing to observe.*alive", None),
    ('observe-statement-outside-scratch-refused', ['host', 'observe-statement-outside-scratch-refused'],
     "module 'game:wrong@0\\.1\\.0' did not load: observe: what it states lies outside the scratch arena.*alive", None),
    ('bridge_observer_statement_t-events-unset-refused', ['host', 'bridge_observer_statement_t-events-unset-refused'],
     "module 'game:wrong@0\\.1\\.0' did not load: observe: its events are unset, or name one there is not.*alive", None),
    ('bridge_term_t-access-unset-refused', ['host', 'bridge_term_t-access-unset-refused'],
     "module 'game:wrong@0\\.1\\.0' did not load: term 0 of system 'again': its access is unset.*alive", None),
    ('bridge_term_t-access-neither-refused', ['host', 'bridge_term_t-access-neither-refused'],
     "module 'game:wrong@0\\.1\\.0' did not load: term 0 of system 'again': its access is 3, and there is read and write.*alive", None),
    ('bridge_term_t-operator-unset-refused', ['host', 'bridge_term_t-operator-unset-refused'],
     "module 'game:wrong@0\\.1\\.0' did not load: term 0 of system 'again': its operator is unset.*alive", None),
    ('bridge_term_t-slot-undeclared-refused', ['host', 'bridge_term_t-slot-undeclared-refused'],
     "module 'game:wrong@0\\.1\\.0' did not load: term 0 of system 'again' names slot 9, which it never declared.*alive", None),
    ('BRIDGE_WRITE-ungranted-refused', ['host', 'BRIDGE_WRITE-ungranted-refused'],
     "module 'game:wrong@0\\.1\\.0' did not load: system 'again' may not write 'other:Thing': no grant says so.*alive", None),
    ('add_system-change-of-an-undeclared-slot-refused', ['host', 'add_system-change-of-an-undeclared-slot-refused'],
     "module 'game:wrong@0\\.1\\.0' did not load: change 0 of system 'again' names slot 9, which it never declared.*alive", None),
    ('add_system-change-ungranted-refused', ['host', 'add_system-change-ungranted-refused'],
     "module 'game:wrong@0\\.1\\.0' did not load: system 'again' may not attach or detach 'other:Thing': no grant says so.*alive", None),
    ('alloc-stated-as-none-refused', ['host', 'alloc-stated-as-none-refused'],
     "module 'game:wrong@0\\.1\\.0' did not load: alloc: it stated that it keeps nothing in the persistent arena.*alive", None),
    ('declare-outside-init-refused', ['host', 'declare-outside-init-refused'],
     "system 'act' failed: declare is called outside init.*alive", None),
    ('add_system-outside-init-refused', ['host', 'add_system-outside-init-refused'],
     "system 'act' failed: add_system is called outside init.*alive", None),
    ('observe-outside-init-refused', ['host', 'observe-outside-init-refused'],
     "system 'act' failed: observe is called outside init.*alive", None),
    ('attach-undeclared-slot-refused', ['host', 'attach-undeclared-slot-refused'],
     "system 'act' failed: attach names slot 9, which it never declared.*alive", None),
    ('attach-ungranted-refused', ['host', 'attach-ungranted-refused'],
     "system 'act' failed: it may not attach 'other:Thing': no grant says so.*alive", None),
    ('attach-value-outside-scratch-refused', ['host', 'attach-value-outside-scratch-refused'],
     "system 'act' failed: attach: the value lies outside the scratch arena.*alive", None),
    ('detach-undeclared-slot-refused', ['host', 'detach-undeclared-slot-refused'],
     "system 'act' failed: detach names slot 9, which it never declared.*alive", None),
    ('module_config.spans-outgrown-at-a-barrier', ['host', 'module_config.spans-outgrown-at-a-barrier'],
     "module 'game:mover@0\\.1\\.0' is stopped: what its queries match no longer fits in its spans of 544 bytes.*alive", None),
    ('BRIDGE_ACC-the-accessors-merged-in', ['nocall', 'mover.wasm', 'move'],
     None, None),
    ('BRIDGE_ERR-nothing-above-an-error-written', ['host', 'BRIDGE_ERR-nothing-above-an-error-written'],
     'record.3.: an error\\s+alive', 'a warning'),
    ('BRIDGE_WARN-a-warning-and-what-is-worse', ['host', 'BRIDGE_WARN-a-warning-and-what-is-worse'],
     'an error.*a warning\\s+alive', 'some information'),
    ('BRIDGE_INFO-information-and-what-is-worse', ['host', 'BRIDGE_INFO-information-and-what-is-worse'],
     'an error.*a warning.*some information\\s+alive', 'a detail'),
    ('BRIDGE_DEBUG-every-line-written', ['host', 'BRIDGE_DEBUG-every-line-written'],
     'an error.*a warning.*some information.*a detail.*alive', None),
    ('BRIDGE_LOG_ERR-written-as-severity-3', ['host', 'BRIDGE_LOG_ERR-written-as-severity-3'],
     'record.3.: an error.*alive', None),
    ('BRIDGE_LOG_WARN-written-as-severity-4', ['host', 'BRIDGE_LOG_WARN-written-as-severity-4'],
     'record.4.: a warning.*alive', None),
    ('BRIDGE_LOG_INFO-written-as-severity-6', ['host', 'BRIDGE_LOG_INFO-written-as-severity-6'],
     'record.6.: some information.*alive', None),
    ('BRIDGE_LOG_DEBUG-written-as-severity-7', ['host', 'BRIDGE_LOG_DEBUG-written-as-severity-7'],
     'record.7.: a detail.*alive', None),
    ('BRIDGE_DECLARE_TAG-carried-by-two-of-three', ['host', 'BRIDGE_DECLARE_TAG-carried-by-two-of-three'],
     'frozen.*alive', None),
    ('bridge_take_scratch-more-than-given-stops-the-module', ['host', 'bridge_take_scratch-more-than-given-stops-the-module'],
     "module 'game:mover@0\\.1\\.0' did not load: its init trapped: wasm `unreachable` instruction executed.*alive", None),
    ('bridge_list_modules-fills-no-more-than-asked', ['host', 'bridge_list_modules-fills-no-more-than-asked'],
     '2 loaded, and of two rows: test:ok@0\\.1\\.0, untouched\\s+alive', None),
    ('bridge_load-import-nobody-answers', ['host', 'bridge_load-import-nobody-answers'],
     "module 'test:stray@0\\.1\\.0' did not load: cannot instantiate: unknown import: `elsewhere::nothing` has not been defined.*alive", None),
    ('bridge_value_t-a-value-of-each-kind', ['host', 'bridge_value_t-a-value-of-each-kind'],
     'vsync width 1 1280.*scale mode 150 2.*show: hello 7 0.*show: some/long/path.cfg 0 0.*flags ratio 3 3.*show: big 2 0.*alive', None),
    ('setting.v-seeds-the-field-its-key-names', ['host', 'setting.v-seeds-the-field-its-key-names'],
     'vsync width 1 1280.*alive', None),
    ('artifact.len-as-many-bytes-compiled', ['host', 'artifact.len-as-many-bytes-compiled'],
     'test:ok@0.1.0: loaded.*alive', None),
    ('record.identity-whose-it-is', ['host', 'record.identity-whose-it-is'],
     'view:reader@0\\.1\\.0/sum: sum 250500 0.*alive', None),
    ('record.system-which-system-said-it', ['host', 'record.system-which-system-said-it'],
     'game:mover@0\\.1\\.0/count: moved.*view:reader@0\\.1\\.0/sum: saw.*alive', None),
    ('record.text-what-was-said', ['host', 'record.text-what-was-said'],
     '/sum: sum 250500 0.*alive', None),
    ('record.len-as-long-as-the-text', ['host', 'record.len-as-long-as-the-text'],
     '/say: kept in the arena 0 0\\s+alive', None),
    ('record.when-the-barrier-it-was-said-after', ['host', 'record.when-the-barrier-it-was-said-after'],
     'b1 \\S+ event 3 1.*b3 \\S+ event 2 2.*b5 \\S+ event 4 57005.*alive', None),
    ('record.arg-the-two-numbers', ['host', 'record.arg-the-two-numbers'],
     'moved 2 1000.*saw 1 500.*alive', None),
    ('module_info.identity-of-each-that-is-loaded', ['host', 'module_info.identity-of-each-that-is-loaded'],
     'modules: game:twostep@0\\.1\\.0=RUNNING test:flaky@0\\.1\\.0=STOPPED .2..*alive', None),
    ('module_info.state-running-and-stopped', ['host', 'module_info.state-running-and-stopped'],
     'modules: game:twostep@0\\.1\\.0=RUNNING test:flaky@0\\.1\\.0=STOPPED .2..*alive', None),
    ('bridge_list_modules-what-is-loaded-and-whether-it-runs', ['host', 'bridge_list_modules-what-is-loaded-and-whether-it-runs'],
     'modules: game:twostep@0\\.1\\.0=RUNNING test:flaky@0\\.1\\.0=STOPPED .2..*alive', None),
    ('bridge_trigger_t-who-triggered-and-how', ['host', 'bridge_trigger_t-who-triggered-and-how'],
     'any: event 3 1.*any: event 2 2.*any: event 4 57005.*alive', None),
    ('bridge_text_t-a-string-a-component-keeps', ['host', 'bridge_text_t-a-string-a-component-keeps'],
     'game:namer@0.1.0/say: kept in the arena.*alive', None),
    ('bridge_log_record_t-written-by-a-module-and-drained', ['host', 'bridge_log_record_t-written-by-a-module-and-drained'],
     'sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive', None),
    ('bridge_line_t-a-line-of-text-found-at-run-time', ['host', 'bridge_line_t-a-line-of-text-found-at-run-time'],
     'game:namer@0.1.0/say: kept in the arena.*alive', None),
    ('BRIDGE_DECLARE-an-enum-and-a-bitmask', ['host', 'BRIDGE_DECLARE-an-enum-and-a-bitmask'],
     'read: body 2 2.*read: more 3 4.*alive', None),
    ('BRIDGE_COMPONENT-declared-by-two-modules', ['host', 'BRIDGE_COMPONENT-declared-by-two-modules'],
     'sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive', None),
    ('BRIDGE_ADD_SYSTEM-registered-and-run', ['host', 'BRIDGE_ADD_SYSTEM-registered-and-run'],
     'sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive', None),
    ('BRIDGE_PUT_CHANGES-an-observer-that-attaches', ['host', 'BRIDGE_PUT_CHANGES-an-observer-that-attaches'],
     'b1 game:chain@0.1.0/got_a: hop A.*b2 game:chain@0.1.0/got_b: hop B.*b3 game:chain@0.1.0/got_c: hop C.*alive', 'hop A.*hop A'),
    ('BRIDGE_ATTACH-the-value-travels-with-it', ['host', 'BRIDGE_ATTACH-the-value-travels-with-it'],
     'sum 250500 0.*alive', None),
    ('BRIDGE_GET-a-field-read', ['host', 'BRIDGE_GET-a-field-read'],
     'sum 250500 0.*alive', None),
    ('BRIDGE_SET-a-field-written-each-frame', ['host', 'BRIDGE_SET-a-field-written-each-frame'],
     'sum 250500 0.*sum 251000 0.*sum 251500 0.*alive', None),
    ('BRIDGE_IMPORT-the-entries-imported', ['host', 'BRIDGE_IMPORT-the-entries-imported'],
     'sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive', None),
    ('BRIDGE_EXPORT-a-system-found-by-its-name', ['host', 'BRIDGE_EXPORT-a-system-found-by-its-name'],
     'sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive', None),
    ('BRIDGE_HEADER-the-scratch-arena-found', ['host', 'BRIDGE_HEADER-the-scratch-arena-found'],
     'game:mover@0.1.0: loaded.*moved.*alive', 'not loaded'),
    ('BRIDGE_STATE-a-statement-put-in-scratch', ['host', 'BRIDGE_STATE-a-statement-put-in-scratch'],
     'sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive', None),
    ('bridge_put-bytes-copied-into-scratch', ['host', 'bridge_put-bytes-copied-into-scratch'],
     'sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive', None),
    ('BRIDGE_LOG-a-line-with-two-numbers', ['host', 'BRIDGE_LOG-a-line-with-two-numbers'],
     'moved 2 1000.*alive', None),
    ('bridge_log-a-line-of-text-found-at-run-time', ['host', 'bridge_log-a-line-of-text-found-at-run-time'],
     'game:namer@0.1.0/say: kept in the arena.*alive', None),
    ('bridge_count_spans-one-table-matched', ['host', 'bridge_count_spans-one-table-matched'],
     'saw 1 500.*alive', None),
    ('bridge_count_rows-rows-of-a-span', ['host', 'bridge_count_rows-rows-of-a-span'],
     'saw 1 500.*alive', None),
    ('bridge_get_base-where-a-column-begins', ['host', 'bridge_get_base-where-a-column-begins'],
     'sum 250500 0.*alive', None),
    ('bridge_get_entity-the-id-of-a-row', ['host', 'bridge_get_entity-the-id-of-a-row'],
     'game:aim@0.1.0/reached: aims 10 5.*game:aim@0.1.0/reached: aimed 20 0.*alive', 'did not load'),
    ('bridge_get_entities-where-a-span-keeps-its-ids', ['host', 'bridge_get_entities-where-a-span-keeps-its-ids'],
     'game:aim@0.1.0/reached: aims 10 5.*game:aim@0.1.0/reached: aimed 20 0.*alive', 'did not load'),
    ('bridge_get_cells-where-the-cells-are', ['host', 'bridge_get_cells-where-the-cells-are'],
     'game:aim@0.1.0/reached: aims 10 5.*game:aim@0.1.0/reached: aimed 20 0.*alive', 'did not load'),
    ('bridge_get_cell-the-cell-of-an-id', ['host', 'bridge_get_cell-the-cell-of-an-id'],
     'game:aim@0.1.0/reached: aims 10 5.*game:aim@0.1.0/reached: aimed 20 0.*alive', 'did not load'),
    ('bridge_get_sparse_index-where-the-index-is', ['host', 'bridge_get_sparse_index-where-the-index-is'],
     'game:hit@0.1.0/walked: hits 4 18.*game:hit@0.1.0/reached: bodies 10 4.*game:hit@0.1.0/reached: reached 18 0.*alive', 'did not load'),
    ('bridge_get_sparse_value-the-value-of-an-id', ['host', 'bridge_get_sparse_value-the-value-of-an-id'],
     'game:hit@0.1.0/walked: hits 4 18.*game:hit@0.1.0/reached: bodies 10 4.*game:hit@0.1.0/reached: reached 18 0.*alive', 'did not load'),
    ('bridge_get_single-where-the-one-row-is', ['host', 'bridge_get_single-where-the-one-row-is'],
     'vsync width 1 1280.*scale mode 150 2.*show: hello 7 0.*show: some/long/path.cfg 0 0.*flags ratio 3 3.*show: big 2 0.*alive', None),
    ('bridge_is_alive-ids-a-sweep-left', ['host', 'bridge_is_alive-ids-a-sweep-left'],
     'left 10 5.*game:keeper@0.1.0: unloaded.*left 9 0.*game:retype@0.1.0: loaded.*alive', None),
    ('bridge_count_triggers-how-many-this-barrier', ['host', 'bridge_count_triggers-how-many-this-barrier'],
     'any: event 3 1.*any: event 2 2.*any: event 4 57005.*alive', None),
    ('bridge_get_trigger-each-in-turn', ['host', 'bridge_get_trigger-each-in-turn'],
     'any: event 3 1.*any: event 2 2.*any: event 4 57005.*alive', None),
    ('bridge_get_trigger_id-who-it-was', ['host', 'bridge_get_trigger_id-who-it-was'],
     'event 4 57005.*alive', None),
    ('bridge_get_bits-where-a-toggle-keeps-them', ['host', 'bridge_get_bits-where-a-toggle-keeps-them'],
     'game:shield@0.1.0/count: shields 10 10.*game:shield@0.1.0/count: strength 45 0.*game:shield@0.1.0/count: shields 10 6.*game:shield@0.1.0/count: strength 27 0.*alive', 'did not load'),
    ('bridge_is_on-counted-by-its-bit', ['host', 'bridge_is_on-counted-by-its-bit'],
     'game:shield@0.1.0/count: shields 10 10.*game:shield@0.1.0/count: strength 45 0.*game:shield@0.1.0/count: shields 10 6.*game:shield@0.1.0/count: strength 27 0.*alive', 'did not load'),
    ('bridge_turn-four-turned-off', ['host', 'bridge_turn-four-turned-off'],
     'shields 10 10.*shields 10 6.*alive', 'did not load'),
    ('BRIDGE_SPAN_SIZE-what-a-span-takes-of-the-spans', ['host', 'BRIDGE_SPAN_SIZE-what-a-span-takes-of-the-spans'],
     "module 'view:reader@0.1.0' did not load: its spans of 16 bytes do not hold a column_base of 2 components by \\d+ tables.*moved 2 1000.*alive", None),
    ('BRIDGE_SCHEMA_TEXT-a-descriptor-of-the-type-s-own-text', ['host', 'BRIDGE_SCHEMA_TEXT-a-descriptor-of-the-type-s-own-text'],
     'sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive', None),
    ('BRIDGE_SCHEMA_STRUCT_TYPE-laid-out-as-its-descriptor', ['host', 'BRIDGE_SCHEMA_STRUCT_TYPE-laid-out-as-its-descriptor'],
     'sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive', None),
    ('BRIDGE_SCHEMA_STRUCT_DESC-read-and-agreed-with', ['host', 'BRIDGE_SCHEMA_STRUCT_DESC-read-and-agreed-with'],
     'sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive', None),
    ('BRIDGE_SCHEMA_ENUM_TYPE-of-the-integer-stated', ['host', 'BRIDGE_SCHEMA_ENUM_TYPE-of-the-integer-stated'],
     'read: body 2 2.*read: more 3 4.*alive', None),
    ('BRIDGE_SCHEMA_ENUM_DESC-read-and-agreed-with', ['host', 'BRIDGE_SCHEMA_ENUM_DESC-read-and-agreed-with'],
     'read: body 2 2.*read: more 3 4.*alive', None),
    ('BRIDGE_SCHEMA_BITMASK_TYPE-of-the-integer-stated', ['host', 'BRIDGE_SCHEMA_BITMASK_TYPE-of-the-integer-stated'],
     'read: body 2 2.*read: more 3 4.*alive', None),
    ('BRIDGE_SCHEMA_BITMASK_DESC-read-and-agreed-with', ['host', 'BRIDGE_SCHEMA_BITMASK_DESC-read-and-agreed-with'],
     'read: body 2 2.*read: more 3 4.*alive', None),
    ('backend_import_t-the-two-memories-found', ['host', 'backend_import_t-the-two-memories-found'],
     'sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive', None),
    ('backend_source_t-no-bytes-refused', ['host', 'backend_source_t-no-bytes-refused'],
     "module 'view:reader@0.1.0' did not load: this backend compiles a module from its bytes, and was handed none.*alive", None),
    ('module_list_imports-the-two-memories-lent', ['host', 'module_list_imports-the-two-memories-lent'],
     'sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive', None),
    ('module_destroy-a-failed-load-leaves-nothing', ['host', 'module_destroy-a-failed-load-leaves-nothing'],
     'game:badterm@0.1.0: not loaded.*game:fp@0.1.0: loaded.*alive', None),
    ('instance_destroy-an-unload-gives-its-share-back', ['host', 'instance_destroy-an-unload-gives-its-share-back'],
     'game:namer@0.1.0/say: kept in the arena.*game:namer@0.1.0: unloaded.*game:ticker@0.1.0: loaded.*alive', None),
    ('instance_set_work_limit-a-spinning-system-ended', ['host', 'instance_set_work_limit-a-spinning-system-ended'],
     "test:spin@0.1.0/forever: system 'forever' failed: trapped: all fuel consumed.*still here.*all fuel consumed.*still here.*alive", None),
    ('instance_set_timeout-a-spinning-init-interrupted', ['host', 'instance_set_timeout-a-spinning-init-interrupted'],
     "module 'test:spin@0.1.0' did not load: its init trapped: interrupt.*alive", None),
    ('instance_set_memory_limit-growth-refused', ['host', 'instance_set_memory_limit-growth-refused'],
     "module 'test:grow@0.1.0' did not load: its init answered 7.*alive", None),
    ('BACKEND_IMPORT_OTHER-a-global-nobody-answers', ['host', 'BACKEND_IMPORT_OTHER-a-global-nobody-answers'],
     "module 'test:strayglobal@0\\.1\\.0' did not load: cannot instantiate: unknown import: `elsewhere::number` has not been defined.*alive", None),
    ('BRIDGE_BOOL-seeds-a-bool', ['host', 'BRIDGE_BOOL-seeds-a-bool'],
     'vsync width 1 1280.*alive', None),
    ('BRIDGE_FLOAT-seeds-a-float', ['host', 'BRIDGE_FLOAT-seeds-a-float'],
     'scale mode 150 2.*alive', None),
    ('BRIDGE_RUNNING-a-module-that-still-runs', ['host', 'BRIDGE_RUNNING-a-module-that-still-runs'],
     'game:twostep@0\\.1\\.0=RUNNING.*alive', None),
    ('BRIDGE_STOPPED-a-module-at-its-failures', ['host', 'BRIDGE_STOPPED-a-module-at-its-failures'],
     'test:flaky@0\\.1\\.0=STOPPED.*alive', None),
    ('BRIDGE_HOST_ABI-stated-and-created', ['host', 'BRIDGE_HOST_ABI-stated-and-created'],
     'test:ok@0.1.0: loaded.*alive', None),
    ('BRIDGE_LEVEL_MOST-every-line-written', ['host', 'BRIDGE_LEVEL_MOST-every-line-written'],
     'an error.*a warning.*some information.*a detail.*alive', None),
    ('BRIDGE_STRUCT-a-struct-a-field-names', ['host', 'BRIDGE_STRUCT-a-struct-a-field-names'],
     'read: body 2 2.*read: more 3 4.*alive', None),
    ('BRIDGE_ENUM-an-enum-a-field-names', ['host', 'BRIDGE_ENUM-an-enum-a-field-names'],
     'read: body 2 2.*read: more 3 4.*alive', None),
    ('BRIDGE_BITMASK-a-bitmask-a-field-names', ['host', 'BRIDGE_BITMASK-a-bitmask-a-field-names'],
     'read: body 2 2.*read: more 3 4.*alive', None),
    ('BRIDGE_TABLE-walked-by-its-rows', ['host', 'BRIDGE_TABLE-walked-by-its-rows'],
     'sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive', None),
    ('BRIDGE_READ-a-term-that-reads', ['host', 'BRIDGE_READ-a-term-that-reads'],
     'sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive', None),
    ('BRIDGE_AND-only-the-entities-that-carry-both', ['host', 'BRIDGE_AND-only-the-entities-that-carry-both'],
     'moved 2 1000.*saw 1 500.*alive', None),
    ('BRIDGE_OPTIONAL-a-term-an-entity-may-lack', ['host', 'BRIDGE_OPTIONAL-a-term-an-entity-may-lack'],
     'game:aim@0.1.0/reached: aims 10 5.*game:aim@0.1.0/reached: aimed 20 0.*alive', 'did not load'),
    ('BRIDGE_ON_ENTER-told-with-the-attach-that-made-it', ['host', 'BRIDGE_ON_ENTER-told-with-the-attach-that-made-it'],
     'event 3 1.*alive', None),
    ('BRIDGE_ON_ATTACH-told-alone-of-an-attach-again', ['host', 'BRIDGE_ON_ATTACH-told-alone-of-an-attach-again'],
     'event 2 2.*alive', None),
    ('BRIDGE_ON_LEAVE-told-of-what-was-destroyed', ['host', 'BRIDGE_ON_LEAVE-told-of-what-was-destroyed'],
     'event 4 57005.*alive', None),
    ('BRIDGE_ABI-stated-and-loaded', ['host', 'BRIDGE_ABI-stated-and-loaded'],
     'test:ok@0.1.0: loaded.*alive', None),
    ('BRIDGE_NAMESPACE-where-every-import-is', ['host', 'BRIDGE_NAMESPACE-where-every-import-is'],
     'sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive', None),
    ('BRIDGE_SHARED-the-memory-two-modules-read', ['host', 'BRIDGE_SHARED-the-memory-two-modules-read'],
     'sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive', None),
    ('BRIDGE_PRIVATE-the-memory-of-one-module', ['host', 'BRIDGE_PRIVATE-the-memory-of-one-module'],
     'sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive', None),
    ('BRIDGE_CARRIED-where-a-tag-would-have-a-value', ['host', 'BRIDGE_CARRIED-where-a-tag-would-have-a-value'],
     'frozen.*alive', None),
    ('BRIDGE_ZEROED-attached-with-no-value', ['host', 'BRIDGE_ZEROED-attached-with-no-value'],
     'frozen.*alive', None),
    ('BRIDGE_SPAN_BASE-where-a-span-keeps-its-columns', ['host', 'BRIDGE_SPAN_BASE-where-a-span-keeps-its-columns'],
     'sum 250500 0.*alive', None),
    ('BACKEND_IMPORT_FUNC-an-entry-imported-is-no-memory', ['host', 'BACKEND_IMPORT_FUNC-an-entry-imported-is-no-memory'],
     'sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive', None),
    ('BACKEND_IMPORT_MEMORY-the-two-memories-lent', ['host', 'BACKEND_IMPORT_MEMORY-the-two-memories-lent'],
     'sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive', None),
    ('config.backends-none-refused', ['host', 'config.backends-none-refused'],
     'the deployment configures no backend, and a module is run by one.*bridge_create refused.*alive', None),
    ('attach-tag-with-a-value-refused', ['host', 'attach-tag-with-a-value-refused'],
     "system 'act' failed: attach: 'game:Flag' is a tag, and is handed a value.*alive", None),
    ('BRIDGE_FAIL-refused-before-it-exists-stops-nothing', ['host', 'BRIDGE_FAIL-refused-before-it-exists-stops-nothing'],
     "module 'test:ok@0\\.1\\.0' is refused: no backend named 'native'.*test:image@0\\.1\\.0: loaded.*alive", 'has stopped'),
]
ALONE = ('BRIDGE_ROW-checked-and-unchecked-timed',)


if __name__ == '__main__':
    main()
