## Scaling

This page describes a number of performance and scaling tricks to make it possible to store millions of data points.

### Sorting

The construction algorithm requires sorted input. To be able to create a dictionary out of millions of keys we apply
  external memory sorting. Fortunately "sorting" huge lists is not a problem these days. keyvi uses 
  [TPIE](http://madalgo.au.dk/tpie/) for external merge sort.

Note: Map-Reduce also sorts data using external memory sort, so using Map-Reduce with 1 Reducer would also give you
sorted input.

But Beware: Sorting can be local dependent, e.g. the Unix 'sort' command behaves differently depending on the locale. Data
must be sorted on the byte level (Unix 'sort' with 'LC_ALL=C')

Code: [dictionary_compiler](/keyvi/src/cpp/dictionary/dictionary_compiler.h)

### Minimization

For each state the compiler stores a fingerprint of the state in the hashtable, although a fingerprint is stored in 
12 bytes the hashtable would not fit into main memory if you have lots of keys.

Therefore keyvi uses several hashtables organized by a LRU (Least Recently Used) Cache: 

The 1st hashtable is filled with a limited number of entries, once full a new hash table is created. If the amount of 
hashtables reaches the limit the last hashtable is thrown away.

To keep "good hashes": Each entry of a successful lookup in a lower hashtable will be moved to the top hashtable. Therefore
states which often minimize will stay in memory, while states which do not minimize will be thrown away over time. 

Code: [LRU Cache](/keyvi/src/cpp/dictionary/fsa/internal/lru_generation_cache.h)

### Compilation/Index Performance

Apart from low-level optimizations like avoiding object copies, pooling, short string optimization, good hash function etc., 
keyvi uses some optimization on the algorithm side.

#### Minimization Stop

As described in Construction the FSA is build from 'right to left', minimization only works this way. Once a minimization 
fails it is impossible to minimize the parent state. Therefore we stop minimization of the preceding states once it fails once.
Note: we still store the fingerprints in the hashtable for later minimizations.

Code: [Unpacked State](/keyvi/src/cpp/dictionary/fsa/internal/unpacked_state.h)

Note: The amount of memory is configurable in the compiler. Increasing the limit might improve compression.

#### Packing

Sparse Array Construction is one of the most demanding parts. To speedup compilation we make use of bit vectors, 
sliding windows and the [De Bruijn](http://en.wikipedia.org/wiki/De_Bruijn_sequence) sequence to quickly find spots to pack 
the data, or - if available - intrinsic compiler/CPU functions. 

Code: [BitVector](/keyvi/src/cpp/dictionary/fsa/internal/bit_vector.h)

### Persistence and Loading

keyvi is a index structure, it is persisted on disk. It does not require to be unpacked when loading. Loading means
mirroring the disk data structure in main memory. The keyvi loader uses Shared Memory for that, which means the index
is loaded only once even if multiple processes read it.

Note: keyvi files can be replicated and distributed.
