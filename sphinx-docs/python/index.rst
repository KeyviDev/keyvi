Index
=====

An index is a near-realtime updateable data structure with full CRUD functionality.


.. code-block:: python

   from keyvi.index import Index
   from keyvi.index import ReadOnlyIndex

   index = Index("/tmp/index-directory")
   index.Set("a", "{'b': 3}")
   index.Flush()

   ri = ReadOnlyIndex("/tmp/index-directory")


.. autoclass:: keyvi.index.Index
   :members:


.. autoclass:: keyvi.index.ReadOnlyIndex
   :members:
