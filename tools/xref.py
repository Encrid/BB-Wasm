#!/usr/bin/env python3
"""Every reference resolves: `doc.md`, *Section* to a real heading, and no
"item N" is left behind. A document of the design names no stage, and names
what is not built only where it is stated or in a citation of that. And no
tracked file of ours says a name that was retired.

Catches the failure the restructure makes likely: a pointer that looks right and
resolves to nothing. The item half exists because the register went when stage 0
ended, so a document citing an item points at text nobody can read - and the
number wraps onto the next line often enough that a line-at-a-time grep misses
it. Run from anywhere: it reads the documents where they are.
"""
import glob
import os
import re
import subprocess
import sys

# it reads the documents by their place under docs/, and is run from anywhere
os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'docs'))

# keyed by basename: prose cites `data.md`, not `bridge/data.md`, because a
# reader wants the document's name and not where it sits in the tree.
heads, paths = {}, {}
# Bridge's documents, which are another project's: a citation of one is resolved against the
# list of their headings that is carried here - docs/bridge_headings.txt
BRIDGE = {(d, h.lower()) for d, _, h in (l.partition('\t') for l in open(
    'bridge_headings.txt', encoding='utf-8').read().split('\n')[1:]) if h}
# contributing.md is the method, not the design: it cites closed items on
# purpose, as the worked examples that make each step concrete, and it carries
# a literal `doc.md`, *Section* template.
for full in glob.glob('**/*.md', recursive=True):
    if os.path.basename(full) == 'contributing.md':
        continue
    f = os.path.basename(full)
    paths[f] = full
    heads[f] = set()
    for line in open(paths[f], encoding='utf-8'):
        # a title counts: a document that is one section has only that heading.
        m = re.match(r'#{1,4}\s+(.*?)\s*$', line)
        if m:
            heads[f].add(m.group(1).lower())

bad = 0
for f in sorted(paths):
    raw = open(paths[f], encoding='utf-8').read()
    # citations wrap, so a reference is matched against unwrapped prose. without
    # this, `doc.md`, *A section whose
    # title wrapped* silently matched nothing and a wrong file went unnoticed.
    text = re.sub(r'\n(?=\S)', ' ', raw)

    # item N, across a line break too: the register was the only place an item
    # number meant anything, and it is gone.
    for m in re.finditer(r'[Ii]tem\s+(\d+)', text):
        line = text.count('\n', 0, m.start()) + 1
        print('%-14s:%d -> item %s is closed' % (f, line, m.group(1)))
        bad += 1

    left = {sec for doc, sec in BRIDGE}

    # cross-document: `other.md`, *Section*   (backticked or not)
    for m in re.finditer(r'`?([A-Za-z0-9_]+\.md)`?[^.\n]{0,30}?\*([^*\n]+)\*', text):
        doc, sec = m.group(1), m.group(2).strip().lower()
        if (doc, sec) in BRIDGE:
            continue
        elif doc not in heads:
            print('%-14s -> missing file %s' % (f, doc))
            bad += 1
        elif sec not in heads[doc]:
            print('%-14s -> %s has no heading "%s"' % (f, doc, sec))
            bad += 1

    # same-document: *Section* with no file named. single asterisks only,
    # so **bold** lead-ins are not mistaken for references.
    for m in re.finditer(r'(?<![*\w])\*(?!\*)([A-Z][^*\n]{3,40})\*(?!\*)', text):
        sec = m.group(1).strip().lower().rstrip('.,;')
        if sec in heads[f] or sec in left or any(sec in heads[d] for d in heads):
            continue
        print('%-14s -> dangling "%s"' % (f, m.group(1)))
        bad += 1

# what the code cites, in a comment, in the same form: a heading that was renamed must not leave
# a function pointing at nothing. A comment's lines are joined first, as a paragraph's are.
for full in sorted(glob.glob('../**/*.[ch]', recursive=True)):
    full = full.replace(os.sep, '/')
    if '/build/' in full:
        continue
    text = re.sub(r'\n[ \t]*(?:\*\*?|//)[ \t]?', ' ', open(full, encoding='utf-8').read())
    for m in re.finditer(r'`?([A-Za-z0-9_]+\.md)`?[^.\n]{0,30}?\*([^*\n]+)\*', text):
        doc, sec = m.group(1), m.group(2).strip().lower()
        if (doc, sec) in BRIDGE:
            continue
        if doc not in heads:
            print('%-14s -> missing file %s' % (full[3:], doc))
            bad += 1
        elif sec not in heads[doc]:
            print('%-14s -> %s has no heading "%s"' % (full[3:], doc, sec))
            bad += 1

# A document of the design names no stage, in any form: the plan points at the design, and the
# design never at the plan - stage14.md, *A stage in a document of the design*. What is held is
# every document that is not the plan's or a record, and the headers and the source. readme.md
# is the index of the tree, the plan's folder with the rest, and style.md gives a stage's file
# as an example of a name: neither says what the library is.
STAGE = re.compile(r'[Ss]tages? \d|stage\d+\.md')
APART = ('plan/', 'refused/')
INDEX = ('plan.md', 'readme.md', 'style.md')


def is_design(path):
    return not path.startswith(APART) and path not in INDEX


for f in sorted(paths):
    full = paths[f].replace(os.sep, '/')
    if not is_design(full):
        continue
    for n, line in enumerate(open(full, encoding='utf-8'), 1):
        if STAGE.search(line):
            print('%-14s:%d names a stage' % (f, n))
            bad += 1
for full in sorted(glob.glob('../sdk/*') + glob.glob('../src/*.[ch]') +
                   glob.glob('../utils/**/*.[ch]', recursive=True)):
    for n, line in enumerate(open(full, encoding='utf-8'), 1):
        if STAGE.search(line):
            print('%s:%d names a stage' % (full[3:].replace(os.sep, '/'), n))
            bad += 1

# Three names were retired, and none comes back - stage15.md, *What holds it*: the name the
# library was built under, what the design before it was called, and the object a host
# creates called by the library's own word. Read in every tracked file of ours that is text.
# The record of the stage that retired them says what each was, and is not read.
RETIRED = [
    (re.compile('ASM' + '700'), 'the name the library was built under'),
    (re.compile(r'\b[Oo]riginal\s+Bridge\b'), 'what the design before this one was called'),
    (re.compile(r"\b(?:[Tt]he|[Aa]|[Aa]n|[Tt]wo|[Nn]o|[Ee]ach|[Ee]very|that|this|its|stopped|"
                r"host's|fixture's)\s+bridges?\b(?![-_/(]|[.:]\w)"),
     'what a host creates, which is a deck'),
]
RECORD = ('docs/bridge_headings.txt',)

listed = subprocess.run(['git', 'ls-files'], cwd='..', stdout=subprocess.PIPE).stdout.decode()
for path in listed.split('\n'):
    if not path or path in RECORD:
        continue
    raw = open(os.path.join('..', path), 'rb').read()
    if b'\0' in raw[:4096]:
        continue
    text = raw.decode('utf-8', 'replace')
    for word, what in RETIRED:
        for m in word.finditer(text):
            print('%s:%d says %s' % (path, text.count('\n', 0, m.start()) + 1, what))
            bad += 1

print('%d broken' % bad)
sys.exit(1 if bad else 0)
