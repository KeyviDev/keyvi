import json
import pykeyvi

from argparse import ArgumentParser

GB = 1024 * 1024 * 1024


def stats(input_file):
    print (json.dumps(pykeyvi.Dictionary(input_file).GetStatistics(), indent=4, sort_keys=True))


def dump(args):
    dictionary = pykeyvi.Dictionary(args.input_file)
    with open(args.output_file, 'w') as file_out:
        for key, value in dictionary.GetAllItems():
            if args.json_dumps:
                key = json.dumps(key)
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
        dictionary = pykeyvi.JsonDictionaryCompiler(GB, params)
    elif dict_type == 'key-only':
        dictionary = pykeyvi.KeyOnlyDictionaryCompiler(GB, params)
    else:
        return 'Must never reach here'

    with open(args.input_file) as file_in:
        for line in file_in:
            line = line.rstrip('\n')
            try:
                splits = line.split('\t')
                if dict_type == 'key-only':
                    dictionary.Add(splits[0])
                else:
                    dictionary.Add(splits[0], splits[1])
            except:
                print ('Can not parse line: {}'.format(line))

    dictionary.Compile()
    dictionary.WriteToFile(args.output_file)


def main():
    argument_parser = ArgumentParser(description='keyvi')
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
    compile_parser.add_argument('dict_type', type=str, choices=['json', 'key-only'],
                                help='dictionary type')
    compile_parser.add_argument('--param', action='append', default=[], dest='compiler_params',
                                type=lambda kv: kv.split("="),
                                help='parameters for keyvi compiler in format param=value')

    args = argument_parser.parse_args()

    if args.command == 'stats':
        stats(args.input_file)
    elif args.command == 'dump':
        dump(args)
    elif args.command == 'compile':
        compile(args)
    else:
        return 'Must never reach here'


if __name__ == '__main__':
    main()
