## keyvi Crashcourse

### Install

Check [install guide](Using keyvi.md) try:

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
    
 
#### Open the file in ipython

Do:

    import pykeyvi
    d = pykeyvi.Dictionary("compiled.keyvi")
    "keyvi" in d
 
Check questions:

 * How does loading works?
 * What happens if you load multiple times (using different processes)?
 
### Lookup and Extraction

Go to [lookup examples](/pykeyvi/examples/lookup)

Compile cities.tsv and run the tester:
    
    keyvicompiler -i cities.tsv -o cities.keyvi -d key-only
    python text_lookup_tester.py

Try queries like: "Fahrradwerkstatt München", "Berlin Alexanderplatz", "San Francisco Coffee Bar"

#### BYO

Try pykeyvi/scripts/compile_json.py and compile your own JSON

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
    
