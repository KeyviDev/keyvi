#!/usr/bin/env python
from __future__ import annotations

import argparse
import gzip
import queue
import re
import threading
from pathlib import Path

from keyvi.compiler import JsonDictionaryCompiler
from keyvi.util import JumpConsistentHashString

control_chars = "".join(map(chr, range(32)))

control_char_re = re.compile(f"[{re.escape(control_chars)}]")


def remove_control_chars(s):
    return control_char_re.sub("", s)


def compile_worker():
    while True:
        compiler, output = compile_queue.get()
        compiler.compile()
        compiler.write_to_file(output)
        compile_queue.task_done()


compile_queue = queue.Queue()


def compile_file(input_path, output, jobs, shards):
    skipped_keys = 0

    compilers = {}
    for i in range(shards):
        compilers[i] = JsonDictionaryCompiler()

    p = Path(input_path)
    input_files = list(p.iterdir()) if p.is_dir() else [p]

    for input_file in input_files:
        if input_file.suffix == ".gz":
            input_fd = gzip.open(input_file)
        else:
            input_fd = input_file.open()

        with input_fd:
            for line in input_fd:
                try:
                    parts = line.split("\t")
                    key = parts[0]

                    if key != remove_control_chars(key):
                        print(
                            "skip key: "
                            + ":".join(f"{ord(c):02x}" for c in key)
                            + " due to containing control characters"
                        )
                        skipped_keys += 1

                    value = parts[1]

                    shard = JumpConsistentHashString(key, shards)
                    compilers[shard].add(key, value)
                except Exception:
                    print("failed to add: " + line)
        print("Skipped keys " + str(skipped_keys))

    for _i in range(jobs):
        t = threading.Thread(target=compile_worker)
        t.daemon = True
        t.start()

    if shards == 1:
        compile_queue.put((compilers[0], output))
    else:
        for i in range(shards):
            compile_queue.put((compilers[i], output + "-" + str(i)))

    compile_queue.join()


ARGV = [
    ("-i", "--input", str, None, "input file"),
    ("-o", "--output", str, None, "output"),
    ("-b", "--bucket", str, None, "s3 bucket to read from"),
    ("-k", "--s3key", str, None, "s3 key/folder to read from"),
    ("-j", "--jobs", int, 1, "number of parallel jobs"),
    ("-s", "--shards", int, 1, "number of shards"),
]


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Compile json keyvi dictionaries")
    for arg in ARGV:
        parser.add_argument(*arg[0:2], type=arg[2], default=arg[3], help=arg[4])
    args = parser.parse_args()
    if args.input:
        compile_file(args.input, args.output, args.jobs, args.shards)
