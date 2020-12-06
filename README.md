##
[![BuildBadge](https://github.com/KeyviDev/keyvi/workflows/Build%20keyvi/badge.svg)](https://github.com/KeyviDev/keyvi/actions)
[![C++](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](/keyvi/README.md)
[![PythonVersions](https://img.shields.io/pypi/pyversions/keyvi.svg)](https://pypi.python.org/pypi/keyvi/)
[![PythonImpl](https://img.shields.io/pypi/implementation/keyvi.svg)](https://pypi.python.org/pypi/keyvi/)
[![PythonFormat](https://img.shields.io/pypi/format/keyvi.svg)](https://pypi.python.org/pypi/keyvi/)
[![PyPIVersion](https://img.shields.io/pypi/v/keyvi.svg)](https://pypi.python.org/pypi/keyvi/)
[![Coveralls](https://coveralls.io/repos/github/KeyviDev/keyvi/badge.svg?branch=master)](https://coveralls.io/github/KeyviDev/keyvi?branch=master)

##
![Keyvi](/doc/images/keyvi-small.png)

Keyvi - the short form for "Key value index" is a key value store (KVS) optimized for size and lookup speed. The usage of shared memory makes it scalable and resistant. The biggest difference to other stores is the underlying data structure based on [finite state machine](https://en.wikipedia.org/wiki/Finite-state_machine). Storage is very space efficient, fast and by design makes various sorts of approximate matching be it fuzzy string matching or geo highly efficient. The immutable FST data structure can be used stand-alone for static datasets. If you need online writes, you can use keyvi index, a _near realtime index_. The index can be used as embedded key value store, e.g. if you already have a network stack in your application. A out of the box network enabled store is available with [keyvi-server](https://github.com/KeyviDev/keyvi-server).

> This is the continuation of cliqz-oss/keyvi. Keyvi was initially developed at Cliqz by Hendrik Muhs and others. For more information, please refer to https://github.com/cliqz-oss/keyvi

## Introduction
  * [BBuzz2016 talk](https://www.youtube.com/watch?v=GBjisdmHe4g)
  * [Announcement blog post](https://cliqz.com/en/aboutus/blog/keyvi)
  * [Search Meetup Munich Jan 2016](http://www.slideshare.net/HendrikMuhs/keyvi-the-key-value-index-cliqz)
  * [Progscon 2017 talk](https://www.infoq.com/presentations/keyvi)
  * [Search Meetup Munich Apr 2018](https://cdn.jsdelivr.net/gh/KeyviDev/keyvi/doc/presentations/searchmuc_apr_2018/keyvi-presentation-search-meetup-2018.svg#1_0)

## Install

### Quick

Precompiled binary wheels are available for OS X and Linux on [PyPi](https://pypi.python.org/pypi/keyvi). To install use:

    pip install keyvi

### From Source

The core part is a C++ header-only library, but the TPIE 3rdparty library needs to be compiled once. The commandline
tools are also part of the C++ code. For instructions check the [Readme](/keyvi/README.md) file.

For the python extension of keyvi check the [Readme](/python/README.md) file in the python subfolder.


## Usage

  * Howtos
    * [Compiling Dictionaries/Indexes](/doc/usage/Building%20keyvi%20dictionaries.md)
    * Python version of keyvi
      * [Compiling](/doc/usage/Building%20keyvi%20dictionaries%20with%20python.md)
      * [Index](/doc/usage/Keyvi%20Index%20with%20python.md)
  * [Crashcourse](/doc/usage/Crashcourse.md)
  * [Using python keyvi with EMR (mrjob or pyspark)](/doc/usage/Using%20pykeyvi%20in%20EMR.md)  

## Internals
  
  * [Construction Basics](/doc/algorithm/Construction-Basics.md)
  * [Persistence Basics](/doc/algorithm/Persistence-Basics.md)
  * [Minimization](/doc/algorithm/Minimization.md)
  * [Scaling](/doc/algorithm/Scaling.md)
  * [Extensibility](/doc/algorithm/Extensibility.md)

If you like to go deep down in the basics, keyvi is inspired by the following 2 papers:

  * Sparse Array (See Storing a Sparse Table, Robert E. Tarjan et al. http://infolab.stanford.edu/pub/cstr/reports/cs/tr/78/683/CS-TR-78-683.pdf)
  * Incremental, which means minimization is done on the fly (See Incremental Construction of Minimal Acyclic Finite-State Automata, J. Daciuk et al.: http://www.mitpressjournals.org/doi/pdf/10.1162/089120100561601)

## Release procedure
  * [How to make a release](doc/RELEASE_PROCESS.md)
  
## Licence and 3rdparty dependencies

keyvi is licenced under apache license 2.0, see [licence](LICENSE) for details.

In addition keyvi uses 3rdparty libraries which define their own licence. Please check their respective licence. 
The 3rdparty libraries can be found at [keyvi/3rdparty](/keyvi/3rdparty).
