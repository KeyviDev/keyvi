# Keyvi index with python

A keyvi index is a simple key value store that lets you read and write. Because keyvi's data structure is immutable,
writes are not immediate, the index works "near realtime". This means the data is available after a refresh in the
background (default every `1s`) or after an explicit `flush`.

A keyvi index creates so called segments in the background, reads have to lookup data in all segments, that's why
segments get merged. Segments are merged in parallel, a heuristic chooses which segments to merge for good performance.
Eventually a keyvi index consists of 1 segment, which is 1 keyvi file. If you look for more information, you can read
how [Lucene](https://lucene.apache.org/) works, keyvi index works similar to it.

## Getting started

To use a keyvi index import the `keyvi.index` module:

```python
from keyvi.index import Index
```

A keyvi index is created with:

```python
index = Index("test-index")
```

The 1st argument is a directory, this is where all files are stored. The directory does not need to exist but is
created for you, if necessary. Best, keep this directory untouched and let keyvi index do its job. To re-open an
existing index, use the same arguments.

A simple set operation:

```python
index.set("a", "42")
```

and get:

```python
m = index.get("a")
print(m.value)
```

For `set` you pass the key with the first argument and the value, which must be a string, as second argument. `get`
returns a match object if the lookup found something, `None` otherwise. Alternatively `index["a"]` raises a `KeyError`
if the key is not found.

With `bulk_set` you can add several values at once:

```python
index.bulk_set([("c", "value_1"), ("d", "value_2")])
```

With `delete` you can delete keys:

```python
index.delete("d")
```

## Index flush

Written keys are not immediately retrievable, but only after a refresh has happened. This happens automatically, if
you want to force a flush yourself you can call `flush`:

```python
index.flush()
```

## Thread safety / multi processing

You can only have one index that _writes_, within the same python process you can use the same index instance and
write from multiple threads (keyvi internally queues all write requests). If you want to use the index from another
process you can open it read-only:

```python
from keyvi.index import ReadOnlyIndex

index = ReadOnlyIndex("test-index")
```

A read-only index only has read methods, e.g. `get` but no write methods. As for a _read-write_ index, writes are
_near realtime_, which means a write from the process that has the _read-write_ index open is not immediately visible
in the _read-only_ index. First a _flush_ in the _read-write_ and then a _refresh_ in the _read-only_ index is required
to make a change visible. (The default is `1s` for each, which means worst-case a write takes `2s` to be visible).

Because keyvi uses _memory mapping_, opening an index in several processes does not increase memory consumption for the
index, but only requires some negligible small data structures as holders.

See the {doc}`../python/index` API reference for all methods.
