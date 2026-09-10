#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Peter Lemenkov <lemenkov@gmail.com>
# SPDX-License-Identifier: GPL-2.0-or-later
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


def fix_gzip_trailer(data):
    """Return gzip data with a correct CRC32 in its trailer.

    The scene files (.vmx) in the archive are gzip members whose trailer
    holds a bogus CRC32; the 2005 reader never checked it, zlib's gz
    functions do and drop the last block. The deflate payload is intact,
    so only the trailer is rewritten.
    """
    if len(data) < 18 or data[:2] != b'\x1f\x8b' or data[2] != 8:
        return data
    flags = data[3]
    pos = 10
    if flags & 4:
        pos += 2 + struct.unpack_from('<H', data, pos)[0]
    if flags & 8:
        pos = data.index(b'\0', pos) + 1
    if flags & 16:
        pos = data.index(b'\0', pos) + 1
    if flags & 2:
        pos += 2
    inflater = zlib.decompressobj(-15)
    payload = inflater.decompress(data[pos:])
    tail = inflater.unused_data
    if len(tail) < 8:
        return data
    trailer = struct.pack('<II', zlib.crc32(payload) & 0xffffffff, len(payload) & 0xffffffff)
    return data[:len(data) - len(tail)] + trailer + tail[8:]


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
            f.write(fix_gzip_trailer(data[pos:pos + size]))
        written += 1
    print(f'{written} files written to {out}, {skipped} skipped')


if __name__ == '__main__':
    main()
