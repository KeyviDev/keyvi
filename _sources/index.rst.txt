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
* `Search Meetup Munich Jan 2016 <http://www.slideshare.net/HendrikMuhs/keyvi-the-key-value-index-cliqz>`_
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

* Sparse Array (See Storing a Sparse Table, Robert E. Tarjan et al.
  http://infolab.stanford.edu/pub/cstr/reports/cs/tr/78/683/CS-TR-78-683.pdf)
* Incremental, which means minimization is done on the fly (See Incremental Construction of Minimal Acyclic
  Finite-State Automata, J. Daciuk et al.: http://www.mitpressjournals.org/doi/pdf/10.1162/089120100561601)


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
