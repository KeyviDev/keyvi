from __future__ import annotations

import json
import sys
from argparse import ArgumentParser
from pathlib import Path

import keyvi
from keyvi.compiler import (
    CompletionDictionaryCompiler,
    CompletionDictionaryMerger,
    IntDictionaryCompiler,
    IntDictionaryMerger,
    JsonDictionaryCompiler,
    JsonDictionaryMerger,
    KeyOnlyDictionaryCompiler,
    KeyOnlyDictionaryMerger,
    StringDictionaryCompiler,
    StringDictionaryMerger,
)
from keyvi.dictionary import Dictionary

COMPILERS = {
    "json": JsonDictionaryCompiler,
    "string": StringDictionaryCompiler,
    "int": IntDictionaryCompiler,
    "completion": CompletionDictionaryCompiler,
    "key-only": KeyOnlyDictionaryCompiler,
}

MERGERS = {
    "json": JsonDictionaryMerger,
    "string": StringDictionaryMerger,
    "int": IntDictionaryMerger,
    "completion": CompletionDictionaryMerger,
    "key-only": KeyOnlyDictionaryMerger,
}


def stats(input_file):
    sys.stdout.write(
        json.dumps(Dictionary(input_file).statistics(), indent=4, sort_keys=True)
    )
    sys.stdout.write("\n")


def dump(args):
    dictionary = Dictionary(args.input_file)
    with Path(args.output_file).open("w") as file_out:
        for entry_key, entry_value in dictionary.items():
            out_key = json.dumps(entry_key) if args.json_dumps else entry_key
            if isinstance(out_key, bytes):
                out_key = out_key.decode()
            file_out.write(out_key)
            if entry_value:
                out_value = json.dumps(entry_value) if args.json_dumps else entry_value
                file_out.write(f"\t{out_value}")
            file_out.write("\n")


def compile_dict(args):
    params = dict(args.compiler_params)
    compiler = COMPILERS[args.dict_type](params)

    with Path(args.input_file).open() as file_in:
        for raw_line in file_in:
            line = raw_line.rstrip("\n")
            try:
                splits = line.split("\t")
                if args.dict_type == "key-only":
                    compiler.add(splits[0])
                elif args.dict_type in ("int", "completion"):
                    compiler.add(splits[0], int(splits[1]))
                else:
                    compiler.add(splits[0], splits[1])
            except Exception:
                sys.stderr.write(f"Can not parse line: {line}\n")

    compiler.compile()
    compiler.write_to_file(args.output_file)


def merge(args):
    params = dict(args.merger_params)
    merger = MERGERS[args.dict_type](params)

    for f in args.input_files:
        merger.add(f)

    merger.merge(args.output_file)


def main():
    argument_parser = ArgumentParser(description="keyvi")

    argument_parser.add_argument(
        "-v", "--version", action="version", version=keyvi.__version__
    )

    subparsers = argument_parser.add_subparsers(dest="command")

    stats_parser = subparsers.add_parser("stats")
    stats_parser.add_argument("input_file", type=str, metavar="FILE")

    dump_parser = subparsers.add_parser("dump")
    dump_parser.add_argument("input_file", type=str, metavar="FILE")
    dump_parser.add_argument("output_file", type=str, metavar="OUT_FILE")
    dump_parser.add_argument(
        "-j", "--json-dumps", action="store_true", help="wrap values with json.dumps()"
    )

    compile_parser = subparsers.add_parser("compile")
    compile_parser.add_argument("input_file", type=str, metavar="FILE")
    compile_parser.add_argument("output_file", type=str, metavar="OUT_FILE")
    compile_parser.add_argument(
        "dict_type",
        type=str,
        choices=["json", "string", "int", "completion", "key-only"],
        help="dictionary type",
    )
    compile_parser.add_argument(
        "--param",
        action="append",
        default=[],
        dest="compiler_params",
        type=lambda kv: kv.split("="),
        help="parameters for keyvi compiler in format param=value",
    )

    merge_parser = subparsers.add_parser("merge")
    merge_parser.add_argument(
        "-i", "--input-files", nargs="+", dest="input_files", required=True
    )
    merge_parser.add_argument("-o", "--output-file", type=str, required=True)
    merge_parser.add_argument(
        "dict_type",
        type=str,
        choices=["json", "string", "int", "completion", "key-only"],
        help="dictionary type",
    )
    merge_parser.add_argument(
        "--param",
        action="append",
        default=[],
        dest="merger_params",
        type=lambda kv: kv.split("="),
        help="parameters for keyvi merger in format param=value",
    )

    args = argument_parser.parse_args()

    if args.command == "stats":
        stats(args.input_file)
    elif args.command == "dump":
        dump(args)
    elif args.command == "compile":
        compile_dict(args)
    elif args.command == "merge":
        if len(args.input_files) < 2:
            merge_parser.error(
                f"Expecting at least 2 input files, got {len(args.input_files)}"
            )
        merge(args)
    else:
        argument_parser.print_usage()


if __name__ == "__main__":
    main()
