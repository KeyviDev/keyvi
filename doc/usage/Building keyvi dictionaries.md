## Preparations

First you need a keyvi installation.

### The dictionarycompiler tool

The scons install target installs the dictionarycompiler in /usr/local/bin.

dictionarycompiler shall be available now:

    dictionarycompiler -h
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

    dictionarycompiler -i test.txt -o test.keyvi


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

You can use dictionaryinspector to dump the data:

    dictionaryinspector -i test.keyvi -o ../test-dump.txt