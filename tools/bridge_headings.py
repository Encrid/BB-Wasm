#!/usr/bin/env python3
"""Write docs/bridge_headings.txt: every heading of Bridge's documents, and the commit of
Bridge they were read at.

BB-Wasm's documents and comments cite Bridge's, and tools/xref.py may read nothing outside this tree. So what
it resolves such a citation against is carried here, as one file. This is the one tool that is
handed another tree, for one run, when BB-Wasm takes a newer Bridge:

    python tools/bridge_headings.py <path to a checkout of Bridge>

The list says which Bridge this backend was written against.
"""
import glob
import os
import re
import subprocess
import sys

if len(sys.argv) != 2 or not os.path.isdir(os.path.join(sys.argv[1], 'docs')):
    sys.exit('usage: bridge_headings.py <path to a checkout of Bridge>')
bridge = sys.argv[1]
here = os.path.dirname(os.path.abspath(__file__))
commit = subprocess.run(['git', 'rev-parse', 'HEAD'], cwd=bridge, stdout=subprocess.PIPE,
                        check=True).stdout.decode().strip()
dirty = subprocess.run(['git', 'status', '--porcelain', 'docs'], cwd=bridge,
                       stdout=subprocess.PIPE, check=True).stdout.strip()
if dirty:
    sys.exit('the documents of that checkout have changes that are not committed')

rows = []
for full in sorted(glob.glob(os.path.join(bridge, 'docs', '**', '*.md'), recursive=True)):
    name = os.path.basename(full)
    for line in open(full, encoding='utf-8'):
        m = re.match(r'#{1,4}\s+(.*?)\s*$', line)
        if m:
            rows.append('%s\t%s' % (name, m.group(1)))

out = os.path.join(here, '..', 'docs', 'bridge_headings.txt')
with open(out, 'w', encoding='utf-8', newline='\n') as f:
    f.write('Bridge %s\n' % commit)
    f.write('\n'.join(rows) + '\n')
print('%d headings of Bridge at %s' % (len(rows), commit[:7]))
