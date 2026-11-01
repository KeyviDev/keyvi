# Compiling dictionaries with python

The compilers are also available from python:

```python
from keyvi.compiler import KeyOnlyDictionaryCompiler

compiler = KeyOnlyDictionaryCompiler()

# repeat for every key
compiler.add("foo")

# finally compile
compiler.compile()
compiler.write_to_file("/tmp/test.kv")
```

Compilers available in `keyvi.compiler`:

| type       | compiler                      |
| ---------- | ----------------------------- |
| integer    | `IntDictionaryCompiler`       |
| completion | `CompletionDictionaryCompiler`|
| key-only   | `KeyOnlyDictionaryCompiler`   |
| string     | `StringDictionaryCompiler`    |
| json       | `JsonDictionaryCompiler`      |

For dictionaries with values, `add` takes the value as second parameter:

```python
compiler.add("foo", 42)
```

To ensure that you do not run out of disk space while compiling, set `$TMPDIR` to a disk with enough free space.

    export TMPDIR=/mnt/tmp

See the {doc}`../python/compiler` API reference for all compilers and their parameters.
