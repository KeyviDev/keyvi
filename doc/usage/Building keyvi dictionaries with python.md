## The python keyvi compilers

The compiler is also available from python keyvi:

    from keyvi.compiler import KeyOnlyDictionaryCompiler
    compiler = KeyOnlyDictionaryCompiler()

    # repeat for every key
    compiler.Add("foo")
    
    # finally compile
    compiler.Compile()
    compiler.WriteToFile("/tmp/test.kv")

Other available Compiler in `keyvi.compiler`

type              | details
----------------- | --------------------------------------------------------------------------------------------- 
integer           | CompletionDictionaryCompiler
key-only          | KeyOnlyDictionaryCompiler
string            | StringDictionaryCompiler
json              | JsonDictionaryCompiler

For dictionaries with values, Add takes the value as second parameter:

    compiler.Add("foo", 42)
    
To ensure that you do not run out of disk space while compiling, set $TMPDIR to a disk with enough free space.
    
    export TMPDIR=/mnt/tmp

