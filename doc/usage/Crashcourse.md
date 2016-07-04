## keyvi Crashcourse

### Install

After installation you should have:

    keyvicompiler
    keyviinspector
    
Try in ipython:
    
    import pykeyvi
    
### First compile and decompile

Open a text editor and put some keys in there, e.g.
    
    keyvi
    is
    a
    key
    value
    index
    
Compile:

    keyvicompiler -i in -o compiled.keyvi -d key-only
    
Dump:
    
    keyviinspector -i compiled.keyvi -o compiled.out

After dumping, open the file compiled.out in a text editor, it should contain your data. 

Check questions:
 * What is the difference to your input file?
 
#### Open the file in ipython

Do:

    import pykeyvi
    d = pykeyvi.Dictionary("compiled.keyvi")
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

### Lookup and Extraction

Go to [lookup examples](/pykeyvi/examples/lookup)

Compile cities.tsv and run the tester:
    
    keyvicompiler -i cities.tsv -o cities.keyvi -d key-only
    python text_lookup_tester.py

Try queries like: "Fahrradwerkstatt München", "Berlin Alexanderplatz", "San Francisco Coffee Bar"

#### Build your own index

Try pykeyvi/scripts/compile_json.py and compile your own JSON. The format should be:

    key {"city": "Munich", "state": "Germany"}

(put a tab ('\t') between key and value, you can also use keyvicompiler instead, but the idea of this excercise is to use the compiler from the python bindings.)

Check statistics:


     keyviinspector -i your-own.keyvi -s


Check questions:

 * Compare values and unique value, whats the meaning of it?

     
With sharding (for distributed data indexes):

     compile_json.py -i your-input -o your-keyvi.keyvi -s 3

### Completion


Go to [completion examples](/pykeyvi/examples/completion)

#### Prefix completion

Have a look at the files completion-nw.tsv, completion.tsv it basically contains keys and integer values:

Compile and try:

    keyvicompiler -i completion-nw.tsv -o prefix-completion.keyvi 
    python prefix_completion_tester.py

Query: '80s'
 
Now try:    

    keyvicompiler -i completion.tsv -o prefix-completion.keyvi
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

Go to [normalization examples](/pykeyvi/examples/normalization)

Compile with:

    keyvicompiler -i normalization.tsv -o normalization.keyvi -d string
    
and try:

    cat sample.txt | python normalize.py
    
### Simple Statistics

Both Keyvi Inspector and the Python API provide an easy way to obtain the number of keys and values in a keyvi-compiled file. 

Example: If there is a keyvi file `foobar.keyvi` with key-value pairs, one can get simple count statistics as follows:

```
$ keyviinspector -i foobar.keyvi -s

General
{"version":"1","start_state":"42613522","number_of_keys":"1768342","value_store_type":"5","number_of_states":"37309831","manifest":""}

Persistence
{"version":"2","size":"42613783"}

Value Store
{"size":"5684145126","values":"2100578","unique_values":"2100571","__compression":"raw","__compression_threshold":"32"}
```

Similarly, With py-keyvi `d.GetStatistics()` on the keyvi dictionary object `d` can output the same information.