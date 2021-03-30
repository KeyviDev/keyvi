.. keyvi documentation master file

Welcome to keyvi!
=================

Keyvi - the short form for "Key value index" is a key value store (KVS) optimized for size and lookup speed. The usage 
of shared memory makes it scalable and resistant. The biggest difference to other stores is the underlying data 
structure based on `finite state machine <https://en.wikipedia.org/wiki/Finite-state_machine>`_. Storage is very space 
efficient, fast and by design makes various sorts of approximate matching be it fuzzy string matching or geo highly 
efficient. The immutable FST data structure can be used stand-alone for static datasets. If you need online writes, 
you can use keyvi index, a "near realtime index". The index can be used as embedded key value store, e.g. if you 
already have a network stack in your application. A out of the box network enabled store is available with 
`keyvi-server <https://github.com/KeyviDev/keyvi-server>`_.

The core of keyvi is written in C++, binding are available for Rust and Python.


API Documentation
=================

.. toctree::
   :maxdepth: 2
   :caption: C++ API

   cpp/dictionary_compiler
   cpp/index

.. toctree::
   :maxdepth: 2
   :caption: Python API
   
   python/dictionary
   python/index
