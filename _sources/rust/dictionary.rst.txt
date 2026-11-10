Dictionary
==========

The ``Dictionary`` struct provides read-only access to a keyvi dictionary file.

.. code-block:: rust

   use keyvi::dictionary::Dictionary;

   let dict = Dictionary::new("data.kv").unwrap();
   let m = dict.get("my_key");
   println!("{}", m.get_value_as_string());


Dictionary
----------

Read-only access to a compiled keyvi dictionary file. Thread-safe (implements ``Send`` and ``Sync``).

.. describe:: Dictionary::new(filename: &str) -> io::Result<Dictionary>

   Open a dictionary from the given file path.

   Returns an error if the file does not exist or cannot be loaded.

.. describe:: Dictionary::with_loading_strategy(filename: &str, loading_strategy: LoadingStrategy) -> io::Result<Dictionary>

   Open a dictionary with a specific loading strategy.

   Available strategies (from ``keyvi::LoadingStrategy``):

   - ``populate`` -- eagerly load the full dictionary into memory.
   - ``lazy`` -- load pages on demand via memory-mapped I/O.
   - ``populate_lazy`` -- advise the OS to load pages but don't block.

.. describe:: Dictionary::get(&self, key: &str) -> KeyviMatch

   Look up a key. Returns a ``KeyviMatch`` (call ``is_empty()`` to check whether the key was found).

.. describe:: Dictionary::get_all_items(&self) -> KeyviMatchIterator

   Return an iterator over all key-value pairs in the dictionary.

.. describe:: Dictionary::get_prefix_completions(&self, key: &str, cutoff: usize) -> KeyviMatchIterator

   Return completions for the given prefix, up to ``cutoff`` results.

.. describe:: Dictionary::get_fuzzy(&self, key: &str, max_edit_distance: usize) -> KeyviMatchIterator

   Return fuzzy matches within the given edit distance.

.. describe:: Dictionary::get_multi_word_completions(&self, key: &str, cutoff: usize) -> KeyviMatchIterator

   Return multi-word completions for the given prefix, up to ``cutoff`` results.

.. describe:: Dictionary::size(&self) -> usize

   Return the number of entries in the dictionary.

.. describe:: Dictionary::statistics(&self) -> String

   Return dictionary statistics as a JSON string.
