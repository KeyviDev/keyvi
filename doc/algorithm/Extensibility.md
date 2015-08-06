## Extensibility

The keyvi compiler is implemented in C++11 and uses templates to allow customization, like having a different 
persistence layer, different minimization, etc.

The most useful customization are different value types:

### Value types

Keys are always strings, values can be of any type, even nested types. Built-in types at time of writing are no-value, 
integer, strings and json.

Value types have to implement a ["duck-type"](http://en.wikipedia.org/wiki/Duck_typing) interface.

Code: [IValue_Store](/keyvi/src/cpp/dictionary/fsa/internal/ivalue_store.h)

In a nutshell, writing a new value store entails: serialization of the value, the interface to the compiler and the 
deserialization for the lookup.

Note: The compiler interface expects an ID for each unique value, the ID is used for minimization.
