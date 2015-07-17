# vim:set ft=python sw=4 et:

# Copyright 2014, The TPIE development team
#
# This file is part of TPIE.
#
# TPIE is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
# License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with TPIE.  If not, see <http://www.gnu.org/licenses/>

import os
import sys
import struct
import json
import tempfile
import subprocess

usage = """Available commands:

read <filename>

    Reads the TPIE stream header in <filename> and dumps it to stdout
    as a JSON dictionary.

write <filename>

    Reads a JSON dictionary from stdin in the format returned by the read
    command and uses it to overwrite the stream header in <filename>.

edit <filename>

    Reads the stream header from <filename> and edits it using your editor (as
    specified by $EDITOR, falling back to $VISUAL), and writes it back out to
    the stream if it was changed.
"""

# From stream_header.h
field_names = [line.split()[1].rstrip(';') for line in """
        uint64_t magic;
        uint64_t version;
        uint64_t itemSize;
        uint64_t blockSize;
        uint64_t userDataSize;
        uint64_t maxUserDataSize;
        uint64_t size;
        uint64_t flags;
        uint64_t lastBlockReadOffset;
        """.strip().splitlines()]

# Everything is a uint64_t, so use Q for all fields
stream_header = struct.Struct('=' + 'q' * len(field_names))

def read(file_name, output_file):
    """Read the TPIE stream in `file_name` (str),
    and print its header as JSON to `output_file` (file)."""
    with open(file_name, 'rb') as stream:
        values = stream_header.unpack_from(stream.read(stream_header.size))
    header_data = dict(zip(field_names, values))
    json.dump(header_data, output_file, indent=0, sort_keys=True, separators=(',', ': '))
    output_file.write('\n')
    output_file.flush()

def write(file_name, input_file):
    """Read a TPIE stream header from `input_file` as JSON,
    and use it to overwrite the file in `file_name`."""
    header_data = json.load(input_file)
    try:
        values = [header_data.pop(key) for key in field_names]
    except (TypeError, KeyError):
        raise SystemExit("Input object incomplete.\n"
            "It should be a dict containing keys\n%r\n" % field_names)
    if header_data:
        raise SystemExit("Input dictionary contains unknown keys.\n"
            "It should contain exactly the keys\n%r\n" % field_names)
    with open(file_name, 'r+b') as stream:
        header_string = stream_header.pack(*values)
        stream.write(header_string)

def main(mode, file_name):
    if mode == 'read':
        read(file_name, sys.stdout)
        sys.stderr.write(
                "Dumped header data. Edit it and use the 'write' command "
                "to overwrite the stream header.\n")
    elif mode == 'write':
        write(file_name, sys.stdin)
    elif mode == 'edit':
        editor = os.environ.get('EDITOR')
        if editor is None:
            editor = os.environ.get('VISUAL')
        if editor is None:
            editor = 'vim'
            sys.stderr.write('No editor specified in $EDITOR or $VISUAL; '
                    'defaulting to %r\n' % editor)
            sys.stderr.flush()

        with tempfile.NamedTemporaryFile(mode='w+', suffix=".json") as tf:
            read(file_name, tf)
            tf.seek(0)
            orig = tf.read()
            tf.seek(0)
            sys.stderr.write('Launching your editor...\n')
            sys.stderr.flush()
            subprocess.call((editor, tf.name))
            changed = tf.read()
            if orig != changed and changed.strip() != '':
                tf.seek(0)
                write(file_name, tf)
                sys.stderr.write('Updated %r\n' % file_name)
            else:
                sys.stderr.write('Nothing was changed; '
                        'nothing written back to file.\n')
    else:
        raise SystemExit(usage)

if __name__ == '__main__':
    try:
        mode, file_name = sys.argv[1:]
    except ValueError:
        raise SystemExit(usage)
    main(*sys.argv[1:])
