## Index using python

A keyvi index is a simple key value store that lets you read and write. As keyvi's data structure is immutable, writes are not
immediate, but "near realtime". This means the data is available after a refresh in the background or after an explicit `Flush`.

A keyvi index creates so called segments in the background, reads have to lookup data in all segments, that's why segments get
merged. Eventually a keyvi index consists of 1 segment, which is 1 keyvi file.

The way it works is similar to [Lucene](https://lucene.apache.org/).

### Getting started

To use a keyvi index import the `keyvi.index` module:

```
import keyvi.index
```

A keyvi index is created with:

```
index = keyvi.index.Index("test-index")
```

The 1st argument is a directory, this is where all files are stored. The directory does not need to exist but is created for you,
if necessary. To re-open an existing index, use the same arguments.

Now you can 
