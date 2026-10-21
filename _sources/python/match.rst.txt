Match
=====

A Match represents a single result from a keyvi lookup.

.. code-block:: python

   from keyvi.dictionary import Dictionary

   d = Dictionary("data.kv")
   m = d.get("some_key")
   if m:
       print(m.matched_string, m.value)


.. autoclass:: keyvi.Match
   :members:
