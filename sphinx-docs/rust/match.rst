Match
=====

``KeyviMatch`` represents a single lookup result from a dictionary.

.. code-block:: rust

   use keyvi::dictionary::Dictionary;

   let dict = Dictionary::new("data.kv").unwrap();
   let m = dict.get("my_key");

   if !m.is_empty() {
       println!("key: {}", m.matched_string());
       println!("value: {}", m.get_value_as_string());
       println!("score: {}", m.get_score());
   }


KeyviMatch
----------

A single match result from a dictionary lookup.

Implements ``Into<(String, serde_json::Value)>`` and ``Into<(String, serde_json::Value, f64)>``
for convenient destructuring.

.. describe:: KeyviMatch::is_empty(&self) -> bool

   Return whether this match is empty (key was not found).

.. describe:: KeyviMatch::matched_string(&self) -> String

   Return the matched key.

.. describe:: KeyviMatch::get_value(&self) -> serde_json::Value

   Return the value as a ``serde_json::Value``.

.. describe:: KeyviMatch::get_value_as_string(&self) -> String

   Return the value as a JSON string.

.. describe:: KeyviMatch::get_score(&self) -> f64

   Return the match score (e.g. edit distance for fuzzy matches, weight for completions).

.. describe:: KeyviMatch::get_msgpacked_value(&self) -> Vec<u8>

   Return the raw value as a MessagePack-encoded byte vector.

.. describe:: KeyviMatch::get_msgpacked_value_compressed(&self, compression_algorithm: Compression) -> Vec<u8>

   Return the raw value as a compressed MessagePack-encoded byte vector.

   Available compression algorithms (from ``keyvi::Compression``):

   - ``NO_COMPRESSION``
   - ``ZLIB_COMPRESSION``
   - ``SNAPPY_COMPRESSION``
   - ``ZSTD_COMPRESSION``


KeyviMatchIterator
------------------

An iterator over match results. Implements the standard Rust ``Iterator`` trait
with ``Item = KeyviMatch``.

.. code-block:: rust

   let dict = Dictionary::new("data.kv").unwrap();
   for m in dict.get_prefix_completions("moz", 10) {
       println!("{}: {}", m.matched_string(), m.get_value_as_string());
   }
