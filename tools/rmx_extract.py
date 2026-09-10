#!/usr/bin/env python3
"""Extract a realtech RMX resource archive into a directory tree.

usage: rmx_extract.py NOGRAVITY.RMX OUTDIR [--keep-junk]

The archive is a 4 byte data offset, a gzip member holding the file
table (entry count, table size, one unused word, then 32 byte name +
size + offset records) and the file data laid out back to back.  Names
are DOS style (".\\menu\\catfont.png"); they are written lower case with
forward slashes.  Duplicate names refer to identical data and the game
resolves a name to its first entry, so the first one wins.  Backup files
(.bak), 4DOS descript.ion files and the demo recordings (.dem) are left
out unless --keep-junk is given.
"""
import os
import struct
import sys
import zlib

NAME_LEN = 32
JUNK_EXT = ('.bak', '.dem')


def read_table(data):
    (offset,) = struct.unpack_from('<i', data, 0)
    if offset < 0:
        offset = -offset
        count = struct.unpack_from('<I', data, 4)[0]
        table = data[16:16 + count * (NAME_LEN + 8)]
    else:
        raw = zlib.decompressobj(31).decompress(data[4:])
        count = struct.unpack_from('<I', raw, 0)[0]
        table = raw[12:12 + count * (NAME_LEN + 8)]
    entries = []
    pos = offset
    for i in range(count):
        rec = table[i * (NAME_LEN + 8):(i + 1) * (NAME_LEN + 8)]
        name = rec[:NAME_LEN].split(b'\0')[0].decode('latin-1')
        size = struct.unpack_from('<I', rec, NAME_LEN)[0]
        entries.append((name, pos, size))
        pos += size
    return entries


def clean_name(name):
    name = name.replace('\\', '/').lower()
    while name.startswith('./'):
        name = name[2:]
    return name


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    keep_junk = '--keep-junk' in sys.argv
    data = open(sys.argv[1], 'rb').read()
    out = sys.argv[2]
    seen = set()
    written = skipped = 0
    for name, pos, size in read_table(data):
        rel = clean_name(name)
        if rel in seen:
            continue
        seen.add(rel)
        base = os.path.basename(rel)
        if not keep_junk and (base.endswith(JUNK_EXT) or base == 'descript.ion'):
            skipped += 1
            continue
        path = os.path.join(out, rel)
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, 'wb') as f:
            f.write(data[pos:pos + size])
        written += 1
    print(f'{written} files written to {out}, {skipped} skipped')


if __name__ == '__main__':
    main()
