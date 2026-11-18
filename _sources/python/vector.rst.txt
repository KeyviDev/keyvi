Vector
======

Keyvi vectors provide indexed access to a sequence of values stored in a keyvi file.

.. code-block:: python

   from keyvi.vector import JsonVectorGenerator, JsonVector

   gen = JsonVectorGenerator()
   gen.append('{"a": 1}')
   gen.append('{"a": 2}')
   gen.write_to_file("data.kvv")

   vec = JsonVector("data.kvv")
   print(vec[0])


.. autoclass:: keyvi.vector.JsonVector
   :members:

.. autoclass:: keyvi.vector.StringVector
   :members:

.. autoclass:: keyvi.vector.JsonVectorGenerator
   :members:

.. autoclass:: keyvi.vector.StringVectorGenerator
   :members:
