## Preparations

First you need a keyvi installation.

### The keyvicompiler tool

The scons install target installs the keyvicompiler in /usr/local/bin.

keyvicompiler shall be available now:

    keyvicompiler -h
    dictionary compiler options::
      -h [ --help ]                         Display this help message
      -v [ --version ]                      Display the version number
      -i [ --input-file ] arg               input file
      -o [ --output-file ] arg              output file
      -m [ --memory-limit ] arg             amount of main memory to use
      -d [ --dictionary-type ] arg (=integer)
                                            type of dictionary (integer (default), 
                                            string, key-only)

To compile simply run for example:

    keyvicompiler -i test.txt -o test.kv


#### The different formats

type              | details
----------------- | --------------------------------------------------------------------------------------------- 
integer           | Expects integer values (tab separated from the key)
key-only          | Expects no values, just keys (e.g. for filters like the adult filter)
string            | Expects string values (tab separated from key)
json              | Expects valid json values (dumped as string)


#### Memory Limit

The level of compression can depend on the amount of memory. If you create large dictionaries it might make a difference
 to allow the compiler the usage of more memory. This will not only speedup the sort phase but might also lead to 
 smaller dictionaries in the end. Per default the memory limit is set to 1 GB.
 
Note: The memory limit just sets the amount of memory the keyvi compiler can use for its minimization hashtables, in addition
keyvi dictionary compiler needs more memory to persist the data. 

#### Dumping keyvi dictionaries

You can use keyviinspector to dump the data:

    keyviinspector -i test.kv -o ../test-dump.txt
