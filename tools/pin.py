"""Moves the pin: writes bridge.txt for a commit of Bridge.

    python tools/pin.py <commit>     the commit, and the hash of each archive its release holds
    python tools/pin.py --refused    the case: an archive that is not the one stated is refused

The first needs BRIDGE_TOKEN, as a build that fetches does. It believes what the release serves
on the day it is run; a build believes bridge.txt, and nothing else, after.
"""
import hashlib
import json
import pathlib
import sys
import tempfile

ROOT = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))
import build


def refused():
    """An archive whose bytes are not those stated: the build ends, and opens nothing."""
    _, hashes = build.pinned()
    with tempfile.TemporaryDirectory() as folder:
        build.unpack(b'not what was published', sorted(hashes)[0], hashes, pathlib.Path(folder))


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    if sys.argv[1] == '--refused':
        refused()
        sys.exit('an archive that is not the one stated was opened')
    commit = sys.argv[1]
    if len(commit) != 40 or set(commit) - set('0123456789abcdef'):
        sys.exit('a commit is named whole: 40 characters')
    release = json.loads(build.ask(build.RELEASES + commit[:12], 'application/vnd.github+json'))
    if release['target_commitish'] != commit:
        sys.exit('the release main-%s is of %s' % (commit[:12], release['target_commitish']))
    lines = ['commit ' + commit]
    for asset in sorted(release['assets'], key=lambda a: a['name']):
        if asset['name'].startswith(('bridge-', 'seams-')):
            data = build.ask(asset['url'], 'application/octet-stream')
            lines.append('%s  %s' % (hashlib.sha256(data).hexdigest(), asset['name']))
    if len(lines) != 1 + 2 * len(build.TARGETS):
        sys.exit('the release holds %d archives of the two packages, and there are %d targets'
                 % (len(lines) - 1, len(build.TARGETS)))
    (ROOT / 'bridge.txt').write_text('\n'.join(lines) + '\n', encoding='utf-8', newline='\n')
    print('bridge.txt: Bridge at %s, %d archives' % (commit[:12], len(lines) - 1))


if __name__ == '__main__':
    main()
