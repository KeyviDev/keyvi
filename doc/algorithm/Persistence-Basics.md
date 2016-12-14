## Persistence Introduction

The default persistence is implemented as sparse array (sparse table). 

Code: [sparse_array_persistence](/keyvi/src/cpp/dictionary/fsa/internal/sparse_array_persistence.h)

### Sparse Array in a nutshell

The underlying data structure consists of 2 simple arrays of the same length(not size), a byte array and a 
pointer array(e.g. uint32_t)

![SparseArraySingleState](/doc/images/sparse_array_single_state.png)

A lookup starts at a given offset, a lookup succeeds if the numeric value (e.g. ASCII value) is found in the bucket 
defined by sum of the offset and the numeric value.

![SparseArrayPointer](/doc/images/sparse_array_pointer.png)

This check is required to allow interleaving of state vectors. To save space vectors of states are interleaved:
 
![SparseArrayInterleaved](/doc/images/sparse_array_mixed.png)

Even with a brute-force method that interleaves state vectors yields a very good compression rate.

### Connecting FSA construction with Sparse Array building

At construction a state is written once all outgoing transitions are known, therefore having the complete state vector.
The algorithm tries to find space in the existing sparse array:

![SparseArrayPacking](/doc/images/sparse_array_packing.png)

Code: [sparse_array_building](/keyvi/src/cpp/dictionary/fsa/internal/sparse_array_builder.h)

