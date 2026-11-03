##
[![BuildBadge](https://github.com/KeyviDev/keyvi/workflows/Build%20keyvi/badge.svg)](https://github.com/KeyviDev/keyvi/actions)
[![Coverage](https://codecov.io/gh/KeyviDev/keyvi/branch/master/graph/badge.svg)](https://codecov.io/gh/KeyviDev/keyvi)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](/keyvi/README.md)
[![PythonVersions](https://img.shields.io/pypi/pyversions/keyvi.svg)](https://pypi.python.org/pypi/keyvi/)
[![PythonImpl](https://img.shields.io/pypi/implementation/keyvi.svg)](https://pypi.python.org/pypi/keyvi/)
[![PythonFormat](https://img.shields.io/pypi/format/keyvi.svg)](https://pypi.python.org/pypi/keyvi/)
[![PyPIVersion](https://img.shields.io/pypi/v/keyvi.svg)](https://pypi.python.org/pypi/keyvi/)
[![CratesVersion](https://img.shields.io/crates/v/keyvi)](https://crates.io/crates/keyvi/)

##
![Keyvi](/doc/images/keyvi-small.png)

Keyvi - the short form for "Key value index" is a key value store (KVS) optimized for size and lookup speed. The usage of shared memory makes it scalable and resistant. The biggest difference to other stores is the underlying data structure based on [finite state machine](https://en.wikipedia.org/wiki/Finite-state_machine). Storage is very space efficient, fast and by design makes various sorts of approximate matching be it fuzzy string matching or geo highly efficient. The immutable FST data structure can be used stand-alone for static datasets. If you need online writes, you can use keyvi index, a _near realtime index_. The index can be used as embedded key value store, e.g. if you already have a network stack in your application. A out of the box network enabled store is available with [keyvi-server](https://github.com/KeyviDev/keyvi-server).

## Introduction

  * [BBuzz2016 talk](https://www.youtube.com/watch?v=GBjisdmHe4g)
  * [Search Meetup Munich Jan 2016](http://www.slideshare.net/HendrikMuhs/keyvi-the-key-value-index-cliqz)
  * [Progscon 2017 talk](https://www.infoq.com/presentations/keyvi)
  * [Search Meetup Munich Apr 2018](https://cdn.jsdelivr.net/gh/KeyviDev/keyvi/doc/presentations/searchmuc_apr_2018/keyvi-presentation-search-meetup-2018.svg#1_0)

## Install

### Quick

Precompiled binary wheels are available for OS X and Linux on [PyPi](https://pypi.python.org/pypi/keyvi). To install use:

    pip install keyvi

### From Source

The core part is a C++ header-only library, which can be used stand-alone. For more information check the [Readme](/keyvi/README.md) file in the keyvi subfolder.

The python extension can be compiled standalone, check the [Readme](/python-pybind/README.md) file in the python-pybind subfolder for more information.


## Documentation

The documentation, including usage guides, internals and API docs, is available at
[keyvidev.github.io/keyvi](https://keyvidev.github.io/keyvi/). Its sources can be found in [sphinx-docs](/sphinx-docs).

## Release procedure
  * [How to make a release](doc/RELEASE_PROCESS.md)

## License

keyvi is licensed under Apache License 2.0("ALv2"), see [license](LICENSE) for details, all [3rdparty libraries](/keyvi/3rdparty) ship with their own license. Except Boost, Snappy and zlib all 3rdparty code can be exclusively found in the [3rdparty](/keyvi/3rdparty) folder. The following licenses are used for the 3rdparty code (last updated: `0.5.0`, provided without warranty).

### C++ dependencies

| Dependency                  | License                 |
| --------------------------- | ----------------------- |
| Boost                       | Boost Software License  |
| moodycamel::ConcurrentQueue | Simplified BSD License  |
| md5                         | RSA MD5 License         |
| msgpack-c                   | Boost Software License  |
| RapidJSON                   | MIT License             |
| Snappy                      | BSD                     |
| Zlib                        | Zlib License            |


### Python dependencies

The python version ships with the same 3rdparty dependencies as the C++ code and additionaly depends on:

| Dependency                  | License                      |
| --------------------------- | ---------------------------- |
| msgpack (for python)        | Apache License, Version 2.0  |


## Credits

Thanks go to:

 - [Cliqz](https://cliqz.com/) for sponsoring and [opening keyvi](https://cliqz.com/en/magazine/cliqzs-major-open-source-contribution-keyvi-key-value-index)
