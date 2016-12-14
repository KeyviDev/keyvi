![Keyvi](/doc/images/keyvi-small.png)

Keyvi - the short form for "Key value index" - defines a special subtype of the popular key value store (KVS) technologies. As you can imagine from the name, keyvi is an immutable key value store, therefore an index not a store. Keyvi's strengths: high compression ratio and extreme scalability. So if you need online read/writes keyvi is not for you, however, if your use case is mostly reads and infrequent writes you might be interested in checking keyvi out.

## Build status

| [Linux][lin-link] | [Coveralls][cov-link] |
| :---------------: | :-------------------: |
| ![lin-badge]      | ![cov-badge]          |

[lin-badge]: https://travis-ci.org/cliqz-oss/keyvi.svg?branch=master "Travis build status"
[lin-link]:  https://travis-ci.org/cliqz-oss/keyvi "Travis build status"
[cov-badge]: https://coveralls.io/repos/cliqz-oss/keyvi/badge.svg?branch=master&service=github
[cov-link]:  https://coveralls.io/github/cliqz-oss/keyvi?branch=master

## Introduction
  * [BBuzz2016 talk](https://www.youtube.com/watch?v=GBjisdmHe4g)
  * [Announcement blog post](https://cliqz.com/en/aboutus/blog/keyvi)
  * [Search Meetup Munich Slidedeck](http://www.slideshare.net/HendrikMuhs/keyvi-the-key-value-index-cliqz)

## Install

### Quick

Precompiled binaries for Ubuntu 14.04LTS are available at the [keyvi PPA](https://launchpad.net/~hendrik-muhs/+archive/ubuntu/keyvi). To install use: 

    sudo add-apt-repository ppa:hendrik-muhs/keyvi
    sudo apt-get update
    sudo apt-get install keyvi python-keyvi

### From Source

The core part is a C++ header-only library, but the TPIE 3rdparty library needs to be compiled once. The commandline
tools are also part of the C++ code. For instructions check the [keyvi Readme](https://github.com/cliqz-oss/keyvi/blob/master/keyvi/) file.

For the python extension pykeyvi check the [pykeyvi Readme](https://github.com/cliqz-oss/keyvi/blob/master/pykeyvi/) file in the pykeyvi subfolder.


## Usage

  * Howtos
    * [Compiling Dictionaries/Indexes](@ref keyvi_compilers)
    * Pykeyvi
      * [Compiling](@ref pykeyvi_compilers)
  * [Crashcourse](@ref crashcourse)
  * [Using pykeyvi with EMR (mrjob or pyspark)](@ref keyvi_emr)  

## Development
 
  * [Build/Packaging](@ref packaging_debian)
  
## Internals
  
  * [Construction Basics](@ref construction_basics)
  * [Persistence Basics](@ref persistence)
  * [Minimization](@ref minimization)
  * [Scaling](@ref scaling_of_construction)
  * [Extensibility](@ref extensibility)

If you like to go deep down in the basics, keyvi is inspired by the following 2 papers:

  * Sparse Array (See Storing a Sparse Table, Robert E. Tarjan et al. http://infolab.stanford.edu/pub/cstr/reports/cs/tr/78/683/CS-TR-78-683.pdf)
  * Incremental, which means minimization is done on the fly (See Incremental Construction of Minimal Acyclic Finite-State Automata, J. Daciuk et al.: http://www.mitpressjournals.org/doi/pdf/10.1162/089120100561601)
  
## Licence and 3rdparty dependencies

keyvi is licenced under apache license 2.0, see [licence](LICENSE) for details.

In addition keyvi uses 3rdparty libraries which define their own licence. Please check their respective licence. 
The 3rdparty libraries can be found at [keyvi/3rdparty](https://github.com/cliqz-oss/keyvi/blob/master/keyvi/3rdparty).
