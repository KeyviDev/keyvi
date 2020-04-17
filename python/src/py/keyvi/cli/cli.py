import json
import keyvi

from argparse import ArgumentParser

from keyvi.compiler import (
    JsonDictionaryCompiler,
    StringDictionaryCompiler,
    IntDictionaryCompiler,
    CompletionDictionaryCompiler,
    KeyOnlyDictionaryCompiler,
    JsonDictionaryMerger,
    StringDictionaryMerger,
    IntDictionaryMerger,
    CompletionDictionaryMerger,
    KeyOnlyDictionaryMerger
)
from keyvi.dictionary import Dictionary


def stats(input_file):
    print(json.dumps(Dictionary(input_file).GetStatistics(), indent=4, sort_keys=True))


def dump(args):
    dictionary = Dictionary(args.input_file)
    with open(args.output_file, 'w') as file_out:
        for key, value in dictionary.GetAllItems():
            if args.json_dumps:
                key = json.dumps(key)
            if isinstance(key, bytes):
                key = key.decode()
            file_out.write(key)
            if value:
                if args.json_dumps:
                    value = json.dumps(value)
                file_out.write('\t{}'.format(value))
            file_out.write('\n')


def compile(args):
    params = {key: value for key, value in args.compiler_params}

    dict_type = args.dict_type
    if dict_type == 'json':
        dictionary = JsonDictionaryCompiler(params)
    elif dict_type == 'string':
        dictionary = StringDictionaryCompiler(params)
    elif dict_type == 'int':
        dictionary = IntDictionaryCompiler(params)
    elif dict_type == 'completion':
        dictionary = CompletionDictionaryCompiler(params)
    elif dict_type == 'key-only':
        dictionary = KeyOnlyDictionaryCompiler(params)
    else:
        return 'Must never reach here'

    with open(args.input_file) as file_in:
        for line in file_in:
            line = line.rstrip('\n')
            try:
                splits = line.split('\t')
                if dict_type == 'key-only':
                    dictionary.Add(splits[0])
                elif dict_type == 'int' or dict_type == 'completion':
                    dictionary.Add(splits[0], int(splits[1]))
                else:
                    dictionary.Add(splits[0], splits[1])
            except:
                print('Can not parse line: {}'.format(line))

    dictionary.Compile()
    dictionary.WriteToFile(args.output_file)


def merge(args):
    params = {key: value for key, value in args.merger_params}

    dict_type = args.dict_type
    if dict_type == 'json':
        merger = JsonDictionaryMerger(params)
    elif dict_type == 'string':
        merger = StringDictionaryMerger(params)
    elif dict_type == 'int':
        merger = IntDictionaryMerger(params)
    elif dict_type == 'completion':
        merger = CompletionDictionaryMerger(params)
    elif dict_type == 'key-only':
        merger = KeyOnlyDictionaryMerger(params)
    else:
        return 'Must never reach here'

    for file in args.input_files:
        merger.Add(file)

    merger.Merge(args.output_file)


def main():
    argument_parser = ArgumentParser(description='keyvi')

    argument_parser.add_argument('-v', '--version', action='version', version=keyvi.__version__)

    subparsers = argument_parser.add_subparsers(dest='command')

    stats_parser = subparsers.add_parser('stats')
    stats_parser.add_argument('input_file', type=str, metavar='FILE')

    dump_parser = subparsers.add_parser('dump')
    dump_parser.add_argument('input_file', type=str, metavar='FILE')
    dump_parser.add_argument('output_file', type=str, metavar='OUT_FILE')
    dump_parser.add_argument('-j', '--json-dumps', action='store_true',
                             help='wrap values with json.dumps()')

    compile_parser = subparsers.add_parser('compile')
    compile_parser.add_argument('input_file', type=str, metavar='FILE')
    compile_parser.add_argument('output_file', type=str, metavar='OUT_FILE')
    compile_parser.add_argument('dict_type', type=str, choices=['json', 'string', 'int', 'completion', 'key-only'],
                                help='dictionary type')
    compile_parser.add_argument('--param', action='append', default=[], dest='compiler_params',
                                type=lambda kv: kv.split("="),
                                help='parameters for keyvi compiler in format param=value')

    merge_parser = subparsers.add_parser('merge')
    merge_parser.add_argument('-i', '--input-files', nargs='+', dest='input_files', required=True)
    merge_parser.add_argument('-o', '--output-file', type=str, required=True)
    merge_parser.add_argument('dict_type', type=str, choices=['json', 'string', 'int', 'completion', 'key-only'],
                              help='dictionary type')
    merge_parser.add_argument('--param', action='append', default=[], dest='merger_params',
                              type=lambda kv: kv.split("="),
                              help='parameters for keyvi merger in format param=value')

    args = argument_parser.parse_args()

    if args.command == 'stats':
        stats(args.input_file)
    elif args.command == 'dump':
        dump(args)
    elif args.command == 'compile':
        compile(args)
    elif args.command == 'merge':
        if len(args.input_files) < 2:
            merge_parser.error('Expecting at least 2 input files, got {}'.format(len(args.input_files)))
        merge(args)
    else:
        argument_parser.print_usage()


if __name__ == '__main__':
    main()
