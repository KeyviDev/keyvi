import json
import pykeyvi

from argparse import ArgumentParser


def main():
    argument_parser = ArgumentParser(description='keyvi')
    subparsers = argument_parser.add_subparsers(dest='command')

    stats_parser = subparsers.add_parser('stats')
    stats_parser.add_argument('input_file', type=str, metavar='FILE')

    args = argument_parser.parse_args()

    if args.command == 'stats':
        print json.dumps(pykeyvi.Dictionary(args.input_file).GetStatistics(), indent=4, sort_keys=True)
    # elif args.command == 'compile':
    #     pass
    else:
        return 'Must never reach here'


if __name__ == '__main__':
    main()
