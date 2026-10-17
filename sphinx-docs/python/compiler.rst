Compiler
========

Compilers are used to create keyvi dictionaries from key-value pairs.
Different compiler types support different value types.

.. code-block:: python

   from keyvi.compiler import JsonDictionaryCompiler

   compiler = JsonDictionaryCompiler()
   compiler.add("key1", '{"value": 1}')
   compiler.add("key2", '{"value": 2}')
   compiler.compile()
   compiler.write_to_file("data.kv")

Compilers can also be used as context managers:

.. code-block:: python

   with JsonDictionaryCompiler() as compiler:
       compiler.add("key1", '{"value": 1}')
       compiler.write_to_file("data.kv")


Compilers
---------

.. autoclass:: keyvi.compiler.JsonDictionaryCompiler
   :members:

.. autoclass:: keyvi.compiler.StringDictionaryCompiler
   :members:

.. autoclass:: keyvi.compiler.IntDictionaryCompiler
   :members:

.. autoclass:: keyvi.compiler.CompletionDictionaryCompiler
   :members:

.. autoclass:: keyvi.compiler.FloatVectorDictionaryCompiler
   :members:

.. autoclass:: keyvi.compiler.KeyOnlyDictionaryCompiler
   :members:


Secondary Key Compilers
-----------------------

.. autoclass:: keyvi.compiler.SecondaryKeyJsonDictionaryCompiler
   :members:

.. autoclass:: keyvi.compiler.SecondaryKeyStringDictionaryCompiler
   :members:

.. autoclass:: keyvi.compiler.SecondaryKeyIntDictionaryCompiler
   :members:

.. autoclass:: keyvi.compiler.SecondaryKeyCompletionDictionaryCompiler
   :members:

.. autoclass:: keyvi.compiler.SecondaryKeyFloatVectorDictionaryCompiler
   :members:

.. autoclass:: keyvi.compiler.SecondaryKeyKeyOnlyDictionaryCompiler
   :members:


Mergers
-------

Mergers combine multiple keyvi dictionaries into one.

.. autoclass:: keyvi.compiler.JsonDictionaryMerger
   :members:

.. autoclass:: keyvi.compiler.StringDictionaryMerger
   :members:

.. autoclass:: keyvi.compiler.IntDictionaryMerger
   :members:

.. autoclass:: keyvi.compiler.CompletionDictionaryMerger
   :members:

.. autoclass:: keyvi.compiler.KeyOnlyDictionaryMerger
   :members:


Generator
---------

.. autoclass:: keyvi.compiler.KeyOnlyDictionaryGenerator
   :members:
