xchange
=======

[![Build Status](https://travis-ci.org/xpol/xchange.png)](https://travis-ci.org/xpol/xchange)

This is a simple project that build a bridge between:

* [msgpack-c](https://github.com/msgpack/msgpack-c)
* [jsoncpp](https://github.com/open-source-parsers/jsoncpp)
* [RapidJSON](https://github.com/miloyip/rapidjson).

Source Code
-----------

The `src/main.cpp` is a demo usage.
The jsoncpp adapter for msgpack-c is `src/msgpack/type/jsoncpp.hpp`.
The RapidJSON adapter for msgpack-c is `src/msgpack/type/rapidjson.hpp`.

Usage
-----

Just check out the `src/main.cpp`.

*Maybe I will document usage here later.*

Current status
--------------

From \ To      | msgpack-c   | jsoncpp             | RapidJSON
---------------|-------------|---------------------|------------
**msgpack-c**  | \-          | Done                | Done
**jsoncpp**    | Done        | \-                  | Done(via msgpack-c)
**RapidJSON**  | Done        | Done(via msgpack-c) | \-

Special Thanks
--------------

* @miloyip
* @pah
