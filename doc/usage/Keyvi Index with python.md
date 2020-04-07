## Keyvi Index with python

A keyvi index is a simple key value store that lets you read and write. Because keyvi's data structure is immutable, writes are not immediate, the index works "near realtime". This means the data is available after a refresh in the background (default every `1s`) or after an explicit `Flush`.

A keyvi index creates so called segments in the background, reads have to lookup data in all segments, that's why segments get merged. Segments are merged in parallel, a heuristic chooses which segments to merge for good performance. Eventually a keyvi index consists of 1 segment, which is 1 keyvi file. If you look for more information, you can read how [Lucene](https://lucene.apache.org/) works, keyvi index uses works similar to it.

### Getting started

To use a keyvi index import the `keyvi.index` module:

```
import keyvi.index
```

A keyvi index is created with:

```
index = keyvi.index.Index("test-index")
```

The 1st argument is a directory, this is where all files are stored. The directory does not need to exist but is created for you, if necessary. Best, keep this directory untouched and let keyvi index do its job. To re-open an existing index, use the same arguments.

A simple set operation:

```
index.Set("a", "42")
```

and get:

```
m = index.Get('a') 
print(m.GetValue())
```

For `Set` you pass the key with the first argument and the value, which must be a string, as second argument. `Get` returns match object if the lookup found something, `None` otherwise.

With `MSet` you can add several values add once:

```
index.MSet([('c', 'value_1'), ('d', 'value_2')])
```

With `Delete` you can delete keys:

```
index.Delete('d')
```

### Index Flush

Written keys are not immediatly retrievable, but only after a refresh has happened. This happens automatically, if you want to force a flush yourself you can call `Flush`:

```
index.Flush()
```

### Thread Safety / Multi Processing

You can only have one index that _writes_, within the same python process you can use the same index instance and write from multiple threads (keyvi internally queues all write requests). If you want to use the index from another process you can open it read-only:

```
index = keyvi.index.ReadOnlyIndex("test-index")
```

A readonly index only has only read methods, e.g. `Get` but no write methods. As for a _read-write_ index, writes are _near realtime_, which means a write from the process that has the _read-write_ index open is not immediatly visible in the _readonly_ index. Firest a _flush_ in the _read-write_ and than a _refresh_ in the _read-only_ index is required to make a change visible. (The default `1s` for each, which means worst-case a write takes `2s` to be visible).

Because keyvi uses _memory mapping_, opening an index in several processes does not increase memory consumption for the index, but only requires some negligible small data structures as holders.
