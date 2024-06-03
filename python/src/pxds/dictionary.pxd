from libcpp.string cimport string as libcpp_string
from libcpp.string cimport string as libcpp_utf8_string
from libcpp.string cimport string as libcpp_utf8_output_string
from libc.stdint cimport int32_t
from libc.stdint cimport uint32_t
from libc.stdint cimport uint64_t
from libcpp cimport bool
from libcpp.pair cimport pair as libcpp_pair
from match cimport Match as _Match
from match_iterator cimport MatchIteratorPair as _MatchIteratorPair
from libcpp.memory cimport shared_ptr

cdef extern from "keyvi/dictionary/dictionary.h" namespace "keyvi::dictionary":
    ctypedef enum loading_strategy_types:
        default_os, # no special treatment, use whatever the OS/Boost has as default
        lazy, # load data as needed with some read-ahead
        populate, # immediately load everything in memory (blocks until everything is fully read)
        populate_key_part, # populate only the key part, load value part lazy
        populate_lazy, # load data lazy but ask the OS to read ahead if possible (does not block)
        lazy_no_readahead, # disable any read-ahead (for cases when index > x * main memory)
        lazy_no_readahead_value_part, # disable read-ahead only for the value part
        populate_key_part_no_readahead_value_part # populate the key part, but disable read ahead value part
        
    cdef cppclass Dictionary:
        # wrap-doc:
        #  Keyvi dictionary, an immutable containers storing key value pairs, optimized
        #  for size, lookup performance and special lookp use cases.
        #  A keyvi dictionary has to be created by a previous compile run.
        #  
        #  Keyvi dictionaries allow multiple types of approximate and completion
        #  matches due to its internal FST based data structure.
        Dictionary (libcpp_utf8_string filename) except +
        Dictionary (libcpp_utf8_string filename, loading_strategy_types) except +
        bool Contains (libcpp_utf8_string key) # wrap-ignore
        shared_ptr[_Match] operator[](libcpp_utf8_string key) # wrap-ignore
        _MatchIteratorPair Get (libcpp_utf8_string key) # wrap-as:match
        _MatchIteratorPair GetNear (libcpp_utf8_string key, size_t minimum_prefix_length) except + # wrap-as:match_near
        _MatchIteratorPair GetNear (libcpp_utf8_string key, size_t minimum_prefix_length, bool greedy) except + # wrap-as:match_near
        _MatchIteratorPair GetFuzzy (libcpp_utf8_string key, int32_t max_edit_distance) except + # wrap-as:match_fuzzy
        _MatchIteratorPair GetFuzzy (libcpp_utf8_string key, int32_t max_edit_distance, size_t minimum_exact_prefix) except + # wrap-as:match_fuzzy
        _MatchIteratorPair GetPrefixCompletion (libcpp_utf8_string key) except + # wrap-as:complete_prefix
        # wrap-doc:
        #  Complete the given key to full matches(prefix matching)
        #  In case the used dictionary supports inner weights, the
        #  completer traverses the dictionary according to weights,
        #  otherwise byte-order.

        _MatchIteratorPair GetPrefixCompletion (libcpp_utf8_string key, size_t top_n) except + # wrap-as:complete_prefix
        # wrap-doc:
        #  Complete the given key to full matches(prefix matching)
        #  and return the top n completions.
        #  In case the used dictionary supports inner weights, the
        #  completer traverses the dictionary according to weights,
        #  otherwise byte-order.
        #  
        #  Note, due to depth-first traversal the traverser
        #  immediately yields results when it visits them. The results are
        #  neither in order nor limited to n. It is up to the caller to resort
        #  and truncate the lists of results.
        #  Only the number of top completions is guaranteed.

        _MatchIteratorPair GetMultiwordCompletion (libcpp_utf8_string key) except + # wrap-as:complete_multiword
        # wrap-doc:
        #  Complete the given key to full matches after whitespace tokenizing.
        #  In case the used dictionary supports inner weights, the
        #  completer traverses the dictionary according to weights,
        #  otherwise byte-order.

        _MatchIteratorPair GetMultiwordCompletion (libcpp_utf8_string key, size_t top_n) except + # wrap-as:complete_multiword
        # wrap-doc:
        #  Complete the given key to full matches after whitespace tokenizing
        #  and return the top n completions.
        #  In case the used dictionary supports inner weights, the
        #  completer traverses the dictionary according to weights,
        #  otherwise byte-order.
        #  
        #  Note, due to depth-first traversal the traverser
        #  immediately yields results when it visits them. The results are
        #  neither in order nor limited to n. It is up to the caller to resort
        #  and truncate the lists of results.
        #  Only the number of top completions is guaranteed.

        _MatchIteratorPair GetFuzzyMultiwordCompletion (libcpp_utf8_string key, int32_t max_edit_distance) except + # wrap-as:complete_fuzzy_multiword
        # wrap-doc:
        #  Complete the given key to full matches after whitespace tokenizing,
        #  allowing up to max_edit_distance distance(Levenshtein).
        #  In case the used dictionary supports inner weights, the
        #  completer traverses the dictionary according to weights,
        #  otherwise byte-order.

        _MatchIteratorPair GetFuzzyMultiwordCompletion (libcpp_utf8_string key, int32_t max_edit_distance, size_t minimum_exact_prefix) except + # wrap-as:complete_fuzzy_multiword
        # wrap-doc:
        #  Complete the given key to full matches after whitespace tokenizing,
        #  allowing up to max_edit_distance distance(Levenshtein) except for
        #  a given exaxt prefix which must match exaxt.
        #  In case the used dictionary supports inner weights, the
        #  completer traverses the dictionary according to weights,
        #  otherwise byte-order.

        _MatchIteratorPair GetAllItems () # wrap-ignore
        _MatchIteratorPair Lookup(libcpp_utf8_string  key) # wrap-as:search
        _MatchIteratorPair LookupText(libcpp_utf8_string text) # wrap-as:search_tokenized
        libcpp_utf8_output_string GetManifest() except + # wrap-as:manifest
        libcpp_string GetStatistics() # wrap-ignore
        uint64_t GetSize() # wrap-ignore
