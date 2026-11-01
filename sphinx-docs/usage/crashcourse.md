# Crashcourse

## Install

    pip install keyvi

After installation you should have the `keyvi` executable available, try:

    keyvi -h

Try in python:

```python
import keyvi
```

Some of the exercises below use the C++ tools `keyvicompiler` and `keyviinspector`, see
{doc}`building-dictionaries` for how to get them.

## First compile and decompile

Open a text editor and put some keys in there, e.g.

    keyvi
    is
    a
    key
    value
    index

Compile:

    keyvi compile <input_file> compiled.kv key-only

Dump:

    keyvi dump compiled.kv dump.out

After dumping, open the file `dump.out` in a text editor, it should contain your data.

Check questions:

- What is the difference to your input file?

### Open the file in python

Do:

```python
from keyvi.dictionary import Dictionary

d = Dictionary("compiled.kv")
"keyvi" in d
```

should return `True`.

```python
match = d["keyvi"]
```

gives you a so called match object.

```python
dir(match)
```

shows all methods you can call on that object, e.g.

```python
match.matched_string
```

should return `"keyvi"`.

Match objects are the common return structure in keyvi. For this very simple example, the other methods are not
useful. But with going deeper into more elaborated examples below, you will find how they can be used.

Check questions:

- How fast does it load? Do you have an idea how loading works internally?
- What happens if you load multiple times (using different processes)?

## Simple statistics

Both, the `keyvi` executable and the Python API provide an easy way to obtain the number of keys and values in a
keyvi file.

For the just created `compiled.kv` file run:

    keyvi stats compiled.kv

The output should be similar to:

```json
{
    "General": {
        "manifest": "",
        "number_of_keys": "6",
        "number_of_states": "14",
        "start_state": "19",
        "value_store_type": "1",
        "version": "2"
    },
    "Persistence": {
        "size": "280",
        "version": "2"
    }
}
```

Note: As we have compiled a `key-only` dictionary there is no info regarding values.

Similarly with python, on the `Dictionary` object `d` the call `d.statistics()` will return the same information.

## Lookup and extraction

Go to the [lookup examples](https://github.com/KeyviDev/keyvi/tree/master/python/examples/lookup).

Compile `cities.tsv` and run the tester:

    keyvicompiler -i cities.tsv -o cities.kv -d key-only
    python text_lookup_tester.py

Try queries like: "Fahrradwerkstatt München", "Berlin Alexanderplatz", "San Francisco Coffee Bar"

### Build your own index

Try `compile_json.py` and compile your own JSON. The format should be:

    key {"city": "Munich", "state": "Germany"}

(put a tab (`\t`) between key and value, you can also use `keyvicompiler` instead, but the idea of this exercise is
to use the compiler from the python bindings.)

Check statistics:

    keyvi stats your-own.kv

Check questions:

- Compare values and unique values, what's the meaning of it?

With sharding (for distributed data indexes):

    python compile_json.py -i your-input -o your-keyvi.kv -s 3

## Completion

Go to the [completion examples](https://github.com/KeyviDev/keyvi/tree/master/python/examples/completion).

### Prefix completion

Have a look at the files `completion-nw.tsv`, `completion.tsv`, they basically contain keys and integer values.

Compile and try:

    keyvicompiler -i completion-nw.tsv -o prefix-completion.kv
    python prefix_completion_tester.py

Query: '80s'

Now try:

    keyvicompiler -i completion.tsv -o prefix-completion.kv
    python prefix_completion_tester.py

Check questions:

- What's the difference between the 2?
- What if you have more data?
- Advanced: How does it work?

### Fuzzy prefix completion

Try:

    python prefix_completion_fuzzy_tester.py

and use misspelled queries like '80s movie wit sombies'

### Multiword completion

Compile with:

    cat completion.tsv | python multiword_completion_writer.py

and try:

    python multiword_completion_tester.py

e.g. 'zombies movie'

Check questions:

- What is the size difference between multiword and pure prefix?
- What is more complex in multiword?
- Why is it still 'relatively small'?

## Normalization

Go to the [normalization examples](https://github.com/KeyviDev/keyvi/tree/master/python/examples/normalization).

Compile with:

    keyvicompiler -i normalization.tsv -o normalization.kv -d string

and try:

    cat sample.txt | python normalize.py
