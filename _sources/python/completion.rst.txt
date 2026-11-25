Completion
==========

Standalone completion classes for more advanced completion use cases.
For simple prefix completion, use :meth:`keyvi.dictionary.Dictionary.complete_prefix` instead.

.. code-block:: python

   from keyvi.dictionary import Dictionary
   from keyvi.completion import PrefixCompletion

   d = Dictionary("completion.kv")
   pc = PrefixCompletion(d)
   for match in pc.complete("hel", 10):
       print(match.matched_string)


.. autoclass:: keyvi.completion.PrefixCompletion
   :members:

.. autoclass:: keyvi.completion.MultiWordCompletion
   :members:

.. autoclass:: keyvi.completion.ForwardBackwardCompletion
   :members:
