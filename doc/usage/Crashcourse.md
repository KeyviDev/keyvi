## keyvi Crashcourse

### Install

After installation you should have `keyvi` executable available:

Try:

    keyvi -h
    
Try in python:
    
    import keyvi
    
### First compile and decompile

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

After dumping, open the file dump.out in a text editor, it should contain your data. 

Check questions:
 * What is the difference to your input file?
 
#### Open the file in python

All python examples assume python 3.

Do:

    from keyvi.dictionary import Dictionary
    d = Dictionary("compiled.kv")
    "keyvi" in d
    
should return True
    
    match = d['keyvi']

gives you a so called match object
    
    dir(match)
 
shows all methods you can call on that object, e.g.

    match.GetMatchedString()
 
should return "keyvi"

Match objects are the common return structure in keyvi. For this very simple example, the other methods are not useful. But with going deeper into more elaborated examples below, you will find how they can be used.
 
Check questions:

 * How fast does it load? Do you have an idea how loading works internally?
 * What happens if you load multiple times (using different processes)?

### Simple Statistics

Both, the `keyvi` executable and the Python API provide an easy way to obtain the number of keys and values in a keyvi file. 

For the just created `compiled.kv` file run:

    keyvi stats compiled.kv

The output should be:

```
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

Note: As we have compiled `key-only` dictionary there is no info regarding values.

Similarly with python, on the `Dictionary` object `d` the call `d.GetStatistics()` will return the same information.

### Lookup and Extraction

Go to [lookup examples](/python/examples/lookup)

Compile cities.tsv and run the tester:
    
    keyvicompiler -i cities.tsv -o cities.kv -d key-only
    python text_lookup_tester.py

Try queries like: "Fahrradwerkstatt München", "Berlin Alexanderplatz", "San Francisco Coffee Bar"

#### Build your own index

Try python/scripts/compile_json.py and compile your own JSON. The format should be:

    key {"city": "Munich", "state": "Germany"}

(put a tab ('\t') between key and value, you can also use keyvicompiler instead, but the idea of this excercise is to use the compiler from the python bindings.)

Check statistics:


     keyviinspector -i your-own.kv -s


Check questions:

 * Compare values and unique value, whats the meaning of it?

     
With sharding (for distributed data indexes):

     compile_json.py -i your-input -o your-keyvi.kv -s 3

### Completion


Go to [completion examples](/python/examples/completion)

#### Prefix completion

Have a look at the files completion-nw.tsv, completion.tsv it basically contains keys and integer values:

Compile and try:

    keyvicompiler -i completion-nw.tsv -o prefix-completion.kv
    python prefix_completion_tester.py

Query: '80s'
 
Now try:    

    keyvicompiler -i completion.tsv -o prefix-completion.kv
    python prefix_completion_tester.py

Check questions:
 
 * What's the difference between the 2?
 * What if you have more data?
 * Advanced: How does it work?

#### Fuzzy Prefix completion

Try:

    python prefix_completion_fuzzy_tester.py
    
and use misspelled queries like '80s movie wit sombies'

#### Multiword Completion

Compile with:

    cat completion.tsv | python multiword_completion_writer.py
    
and try:

    python multiword_completion_tester.py
    
e.g. 'zombies movie'

Check questions:

 * What is the size difference between multiword and pure prefix?
 * What is more complex in multiword?
 * Why is it still 'relative small'?

### Normalization

Go to [normalization examples](/python/examples/normalization)

Compile with:

    keyvicompiler -i normalization.tsv -o normalization.kv -d string
    
and try:

    cat sample.txt | python normalize.py
    
