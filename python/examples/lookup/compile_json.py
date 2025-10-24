#!/usr/bin/env python
# -*- coding: utf-8 -*-

import Queue
import threading
import argparse
import os
import gzip
import unicodedata, re

from keyvi.compiler import JsonDictionaryCompiler
from keyvi.util import JumpConsistentHashString

control_chars = ''.join(map(unichr, range(0,32)))

control_char_re = re.compile('[%s]' % re.escape(control_chars))

def remove_control_chars(s):
    return control_char_re.sub('', s)


def compile_worker():
    while True:
        compiler, output = compile_queue.get()
        compiler.Compile()
        compiler.WriteToFile(output)
        compile_queue.task_done()

compile_queue = Queue.Queue()

def compile_file(input, output, jobs, shards):
    skipped_keys = 0

    compilers = {}
    for i in range (0, shards):
        compilers[i] = JsonDictionaryCompiler()

    if os.path.isdir(input):
        input_files = [os.path.join(input,d) for d in os.listdir(input)]
    else:
        input_files = [input]

    for input_file in input_files:
        if input_file.endswith(".gz"):
            input_fd = gzip.open(input_file)
        else:
            input_fd = open(input_file)

        for line in input_fd:
            try:
                parts = line.split("\t")
                key = parts[0]

                if key != remove_control_chars(key):
                    print("skip key: " + ":".join("{:02x}".format(ord(c)) for c in key) + " due to containing control characters")
                    skipped_keys +=1

                value = parts[1]

                shard = JumpConsistentHashString(key, shards)
                compilers[shard].Add(key, value)
            except:
                print("failed to add: " + line)
        print("Skipped keys " + str(skipped_keys))

    for i in range(jobs):
         t = threading.Thread(target=compile_worker)
         t.daemon = True
         t.start()

    if shards == 1:
        compile_queue.put((compilers[i], output))
    else:
        for i in range (0, shards):
            compile_queue.put((compilers[i], output + "-" + str(i)))

    compile_queue.join()


ARGV = [
    ('-i', '--input',           str, None,      'input file'),
    ('-o', '--output',          str, None,      'output'),
    ('-b', '--bucket',          str, None,      's3 bucket to read from'),
    ('-k', '--s3key',           str, None,      's3 key/folder to read from'),
    ('-j', '--jobs',            int, 1,         'number of parallel jobs'),
    ('-s', '--shards',          int, 1,         'number of shards'),
]


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Compile json keyvi dictionaries')
    for arg in ARGV:
        parser.add_argument(*arg[0:2], type=arg[2], default=arg[3], help=arg[4])
    args = parser.parse_args()
    if args.input:
        compile_file(args.input, args.output, args.jobs, args.shards)
