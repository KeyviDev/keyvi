Keyvi - the short form for "Key value index" is a key value store (KVS) optimized for size and lookup speed. The usage of shared memory makes it scalable and resistant. The biggest difference to other stores is the underlying data structure based on [finite state machine](https://en.wikipedia.org/wiki/Finite-state_machine). Storage is very space efficient, fast and by design makes various sorts of approximate matching be it fuzzy string matching or geo highly efficient. The immutable FST data structure can be used stand-alone for static datasets. If you need online writes, you can use keyvi index, a _near realtime index_.

## Quick Start

Install keyvi with

```
pip3 install keyvi
```

create your first very simple index:

```
import keyvi.index
index = keyvi.index.Index("test-index")

index.Set('key', '{"answer": 42, "condition": "always"}')
index.Flush()
# get the entry for key
m = index.Get('key')
print(m.GetValue())

# match fuzzy(levenshtein distance) with max edit distance 1, exact prefix 2
m = index.GetFuzzy("kei", 1, 2)
print(m.GetMatchedString())
```

For more information visit http://keyvi.org
