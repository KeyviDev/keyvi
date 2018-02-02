##
[![Travis](https://travis-ci.org/KeyviDev/keyvi.svg?branch=master  "Travis build status")](https://travis-ci.org/KeyviDev/keyvi)
[![C++](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](/keyvi/README.md)
[![PythonVersions](https://img.shields.io/pypi/pyversions/keyvi.svg)](https://pypi.python.org/pypi/keyvi/)
[![PythonImpl](https://img.shields.io/pypi/implementation/keyvi.svg)](https://pypi.python.org/pypi/keyvi/)
[![PythonFormat](https://img.shields.io/pypi/format/keyvi.svg)](https://pypi.python.org/pypi/keyvi/)
[![PyPIVersion](https://img.shields.io/pypi/v/keyvi.svg)](https://pypi.python.org/pypi/keyvi/)
[![Coveralls](https://coveralls.io/repos/github/KeyviDev/keyvi/badge.svg?branch=master)](https://coveralls.io/github/KeyviDev/keyvi?branch=master)

##
![Keyvi](/doc/images/keyvi-small.png)

Keyvi - the short form for "Key value index" - defines a special subtype of the popular key value store (KVS) technologies. As you can imagine from the name, keyvi is an immutable key value store, therefore an index not a store. Keyvi's strengths: high compression ratio and extreme scalability. So if you need online read/writes keyvi is not for you, however, if your use case is mostly reads and infrequent writes you might be interested in checking keyvi out.

> This is the continuation of cliqz-oss/keyvi. Keyvi was initially developed at Cliqz by Hendrik Muhs and others. For more information, please refer to https://github.com/cliqz-oss/keyvi

## Introduction
  * [BBuzz2016 talk](https://www.youtube.com/watch?v=GBjisdmHe4g)
  * [Announcement blog post](https://cliqz.com/en/aboutus/blog/keyvi)
  * [Search Meetup Munich Slidedeck](http://www.slideshare.net/HendrikMuhs/keyvi-the-key-value-index-cliqz)
  * [Progscon 2017 talk](https://www.infoq.com/presentations/keyvi)

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
  
## Licence and 3rdparty dependencies

keyvi is licenced under apache license 2.0, see [licence](LICENSE) for details.

In addition keyvi uses 3rdparty libraries which define their own licence. Please check their respective licence. 
The 3rdparty libraries can be found at [keyvi/3rdparty](/keyvi/3rdparty).
