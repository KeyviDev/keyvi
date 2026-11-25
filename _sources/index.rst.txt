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

Install the python version with::

   pip install keyvi

Talks and presentations:

* `BBuzz2016 talk <https://www.youtube.com/watch?v=GBjisdmHe4g>`_
  (`slides <https://cdn.jsdelivr.net/gh/KeyviDev/keyvi/doc/presentations/bbuzz2016/keyvi-presentation.svg#1_0>`__)
* `Search Meetup Munich Jan 2016 <https://www.slideshare.net/slideshow/keyvi-the-key-value-index-cliqz/57212536>`_
* `Progscon 2017 talk <https://www.infoq.com/presentations/keyvi>`_
* `Search Meetup Munich Apr 2018
  <https://cdn.jsdelivr.net/gh/KeyviDev/keyvi/doc/presentations/searchmuc_apr_2018/keyvi-presentation-search-meetup-2018.svg#1_0>`_

.. toctree::
   :maxdepth: 2
   :caption: Usage

   usage/crashcourse
   usage/building-dictionaries
   usage/building-dictionaries-python
   usage/index-python
   usage/emr

.. toctree::
   :maxdepth: 2
   :caption: Internals

   internals/construction-basics
   internals/persistence-basics
   internals/minimization
   internals/scaling
   internals/extensibility

If you like to go deep down in the basics, keyvi is inspired by the following 2 papers:

* Sparse Array (See `Storing a Sparse Table <https://doi.org/10.1145/359168.359175>`_, Robert E. Tarjan et al.,
  `tech report PDF
  <http://bitsavers.informatik.uni-stuttgart.de/pdf/stanford/Stanford_CS_TR_Collection_2025-12-12/PDF/1978/CS-TR-78-683.pdf>`_)
* Incremental, which means minimization is done on the fly (See `Incremental Construction of Minimal Acyclic
  Finite-State Automata <https://doi.org/10.1162/089120100561601>`_, J. Daciuk et al.)


API Documentation
=================

.. toctree::
   :maxdepth: 2
   :caption: C++ API

   cpp/dictionary_compiler
   cpp/index

.. toctree::
   :maxdepth: 2
   :caption: Rust API

   rust/dictionary
   rust/match

.. toctree::
   :maxdepth: 2
   :caption: Python API

   python/match
   python/dictionary
   python/compiler
   python/completion
   python/index
   python/vector
   python/util
