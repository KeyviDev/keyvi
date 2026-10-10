"""
keyvi - A key value store.

Copyright 2026 Hendrik Muhs<hendrik.muhs@gmail.com>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

  http:www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

from __future__ import annotations

import sys
from argparse import ArgumentParser

import keyvi2.compiler

if __name__ == "__main__":
    argument_parser = ArgumentParser(description="keyvi")

    argument_parser.add_argument(
        "-i", "--input-file", action="append", default=[], dest="input_file"
    )
    argument_parser.add_argument("-o", "--output-file", type=str)
    argument_parser.add_argument(
        "-m", "--memory-limit", type=str, help="amount of main memory to use"
    )
    argument_parser.add_argument(
        "-p",
        "--parameter",
        action="append",
        default=[],
        dest="params",
        type=lambda kv: kv.split("="),
        help="An option; format is -p xxx=yyy",
    )

    args = argument_parser.parse_args()

    if args.output_file and len(args.input_file) > 0:
        params = dict(args.params)

        if args.memory_limit:
            params["memory-limit"] = args.memory_limit

        merger = keyvi2.compiler.JsonDictionaryMerger(params)
        for f in args.input_file:
            merger.add(f)

        merger.merge(args.output_file)
    else:
        sys.stderr.write("ERROR: arguments wrong or missing.\n")
        sys.exit(1)

    sys.exit(0)
