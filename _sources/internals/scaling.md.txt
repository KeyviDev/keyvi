# Scaling

This page describes a number of performance and scaling tricks to make it possible to store millions of data points.

## Sorting

The construction algorithm requires sorted input. keyvi uses external memory sorting, it spills to disk when necessary
and therefore works for large data.

Code: [dictionary_compiler](https://github.com/KeyviDev/keyvi/blob/master/keyvi/include/keyvi/dictionary/dictionary_compiler.h)

## Minimization

For each state the compiler stores a fingerprint of the state in the hashtable, although a fingerprint is stored in
12 bytes the hashtable would not fit into main memory if you have lots of keys.

Therefore keyvi uses several hashtables organized by a LRU (Least Recently Used) Cache:

The 1st hashtable is filled with a limited number of entries, once full a new hash table is created. If the amount of
hashtables reaches the limit the last hashtable is thrown away.

To keep "good hashes": Each entry of a successful lookup in a lower hashtable will be moved to the top hashtable. Therefore
states which often minimize will stay in memory, while states which do not minimize will be thrown away over time.

Code: [LRU Cache](https://github.com/KeyviDev/keyvi/blob/master/keyvi/include/keyvi/dictionary/fsa/internal/lru_generation_cache.h)

## Compilation/Index Performance

Apart from low-level optimizations like avoiding object copies, pooling, short string optimization, good hash function etc.,
keyvi uses some optimization on the algorithm side.

### Minimization Stop

As described in Construction the FSA is build from 'right to left', minimization only works this way. Once a minimization
fails it is impossible to minimize the parent state. Therefore we stop minimization of the preceding states once it fails once.
Note: we still store the fingerprints in the hashtable for later minimizations.

Code: [Unpacked State](https://github.com/KeyviDev/keyvi/blob/master/keyvi/include/keyvi/dictionary/fsa/internal/unpacked_state.h)

Note: The amount of memory is configurable in the compiler. Increasing the limit might improve compression.

### Packing

Sparse Array Construction is one of the most demanding parts. To speedup compilation we make use of bit vectors,
sliding windows and the [De Bruijn](http://en.wikipedia.org/wiki/De_Bruijn_sequence) sequence to quickly find spots to pack
the data, or - if available - intrinsic compiler/CPU functions.

Code: [BitVector](https://github.com/KeyviDev/keyvi/blob/master/keyvi/include/keyvi/dictionary/fsa/internal/bit_vector.h)

## Persistence and Loading

keyvi is a index structure, it is persisted on disk. It does not require to be unpacked when loading. Loading means
mirroring the disk data structure in main memory. The keyvi loader uses Shared Memory for that, which means the index
is loaded only once even if multiple processes read it.

Note: keyvi files can be replicated and distributed.
