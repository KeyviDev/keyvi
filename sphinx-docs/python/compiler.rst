Compiler
========

Compilers are used to create keyvi dictionaries from key-value pairs.
Different compiler types support different value types.

.. code-block:: python

   from keyvi.compiler import JsonDictionaryCompiler

   compiler = JsonDictionaryCompiler({"memory_limit_mb": "256"})
   compiler.add("key1", '{"value": 1}')
   compiler.add("key2", '{"value": 2}')
   compiler.compile()
   compiler.write_to_file("data.kv")


Parameters
----------

All compilers and mergers accept a ``params`` dictionary. Values are passed as strings.

Common compiler parameters:

.. list-table::
   :header-rows: 1
   :widths: 30 10 15 45

   * - Parameter
     - Type
     - Default
     - Description
   * - ``memory_limit``
     - int
     - 1 GB
     - Memory limit in bytes. Also accepts ``memory_limit_kb``, ``memory_limit_mb``, ``memory_limit_gb``.
   * - ``temporary_path``
     - str
     - system temp dir
     - Directory for temporary files during compilation.
   * - ``minimization``
     - bool
     - true
     - Enable FSA minimization. Disabling produces larger output but compiles faster.
   * - ``parallel_sort_threshold``
     - int
     - 10000
     - Use parallel sort when key count exceeds this threshold. Set to 0 to disable.

Additional parameters for ``JsonDictionaryCompiler`` and ``FloatVectorDictionaryCompiler``:

.. list-table::
   :header-rows: 1
   :widths: 30 10 15 45

   * - Parameter
     - Type
     - Default
     - Description
   * - ``compression``
     - str
     - none
     - Compression type: ``'zlib'``, ``'snappy'``, ``'zstd'``, or empty for none.
   * - ``compression_threshold``
     - int
     - 32
     - Minimum value size in bytes to apply compression.
   * - ``floating_point_precision``
     - str
     - double
     - JSON only. Set to ``'single'`` for 32-bit floats.
   * - ``vector_size``
     - int
     - 10
     - FloatVector only. Number of dimensions per vector.

Merger-specific parameters:

.. list-table::
   :header-rows: 1
   :widths: 30 10 15 45

   * - Parameter
     - Type
     - Default
     - Description
   * - ``merge_mode``
     - str
     - standard
     - Set to ``'append'`` for append mode.

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
