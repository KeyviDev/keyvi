# Compiling dictionaries

keyvi dictionaries can be compiled from the command line, either with the `keyvi` command that ships with the python
package or with the C++ `keyvicompiler` tool.

## The `keyvi` command line tool

After `pip install keyvi` the `keyvi` command is available:

    keyvi compile input.tsv output.kv key-only

The last argument is the dictionary type, one of `json`, `string`, `int`, `completion` or `key-only`. Compiler
parameters can be passed with `--param`, e.g. `--param memory_limit_mb=2048`.

To dump a dictionary:

    keyvi dump output.kv dump.txt

To show statistics:

    keyvi stats output.kv

## The `keyvicompiler` tool

When building keyvi from source (see the [C++ readme](https://github.com/KeyviDev/keyvi/blob/master/keyvi/README.md)),
the `keyvicompiler` binary is built as well:

    keyvicompiler -h
    keyvi compiler options::
      -h [ --help ]                         Display this help message
      -v [ --version ]                      Display the version number
      -i [ --input-file ] arg               input file
      -o [ --output-file ] arg              output file
      -m [ --memory-limit ] arg             amount of main memory to use
      -d [ --dictionary-type ] arg (=integer)
                                            type of dictionary (integer (default),
                                            string, key-only, json, completion)
      -p [ --parameter ] arg (=EMPTY)       An option; format is -p xxx=yyy
      --manifest arg                        manifest to be embedded

To compile simply run for example:

    keyvicompiler -i test.txt -o test.kv

### Input formats

| type       | details                                                         |
| ---------- | --------------------------------------------------------------- |
| integer    | Expects integer values (tab separated from the key)             |
| key-only   | Expects no values, just keys (e.g. for filters)                 |
| string     | Expects string values (tab separated from key)                  |
| json       | Expects valid json values (dumped as string)                    |
| completion | Expects integer weights (tab separated from the key)            |

### Memory limit

The level of compression can depend on the amount of memory. If you create large dictionaries it might make a
difference to allow the compiler the usage of more memory. This will not only speedup the sort phase but might also lead
to smaller dictionaries in the end. Per default the memory limit is set to 1 GB.

Note: The memory limit just sets the amount of memory the keyvi compiler can use for its minimization hashtables, in
addition keyvi dictionary compiler needs more memory to persist the data.

### Dumping keyvi dictionaries

You can use `keyviinspector` to dump the data:

    keyviinspector -i test.kv -o ../test-dump.txt

or to show statistics:

    keyviinspector -i test.kv -s
