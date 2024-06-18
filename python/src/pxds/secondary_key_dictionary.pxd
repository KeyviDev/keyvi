from libcpp.string cimport string as libcpp_string
from libcpp.string cimport string as libcpp_utf8_string
from libcpp.string cimport string as libcpp_utf8_output_string
from libcpp.map cimport map as libcpp_map
from libc.stdint cimport int32_t
from libc.stdint cimport uint32_t
from libc.stdint cimport uint64_t
from libcpp cimport bool
from libcpp.pair cimport pair as libcpp_pair
from match cimport Match as _Match
from match_iterator cimport MatchIteratorPair as _MatchIteratorPair
from libcpp.memory cimport shared_ptr

cdef extern from "keyvi/dictionary/secondary_key_dictionary.h" namespace "keyvi::dictionary":
    cdef cppclass SecondaryKeyDictionary:
        # wrap-doc:
        #  Secondary Key dictionary, specialized dictionary type that takes additional key value pairs
        #  that are matched exact upfront, enabling use cases like multi-tenancy and personalization.

        SecondaryKeyDictionary (libcpp_utf8_string filename) except +
        #SecondaryKeyDictionary (libcpp_utf8_string filename, loading_strategy_types) except +
        shared_ptr[_Match] GetFirst (libcpp_utf8_string the_key, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] meta) # wrap-ignore
        bool Contains (libcpp_utf8_string the_key, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] meta) # wrap-ignore
        _MatchIteratorPair Get (libcpp_utf8_string the_key, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] meta) # wrap-as:match
        _MatchIteratorPair GetNear (libcpp_utf8_string the_key, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] meta, size_t minimum_prefix_length) except + # wrap-as:match_near
        _MatchIteratorPair GetNear (libcpp_utf8_string the_key, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] meta, size_t minimum_prefix_length, bool greedy) except + # wrap-as:match_near
        _MatchIteratorPair GetFuzzy (libcpp_utf8_string the_key, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] meta, int32_t max_edit_distance) except + # wrap-as:match_fuzzy
        _MatchIteratorPair GetFuzzy (libcpp_utf8_string the_key, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] meta, int32_t max_edit_distance, size_t minimum_exact_prefix) except + # wrap-as:match_fuzzy
        _MatchIteratorPair GetPrefixCompletion (libcpp_utf8_string the_key, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] meta) except + # wrap-as:complete_prefix
        # wrap-doc:
        #  Complete the given key to full matches(prefix matching)
        #  In case the used dictionary supports inner weights, the
        #  completer traverses the dictionary according to weights,
        #  otherwise byte-order.

        _MatchIteratorPair GetPrefixCompletion (libcpp_utf8_string the_key, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] meta, size_t top_n) except + # wrap-as:complete_prefix
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

        _MatchIteratorPair GetMultiwordCompletion (libcpp_utf8_string the_key, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] meta) except + # wrap-as:complete_multiword
        # wrap-doc:
        #  Complete the given key to full matches after whitespace tokenizing.
        #  In case the used dictionary supports inner weights, the
        #  completer traverses the dictionary according to weights,
        #  otherwise byte-order.

        _MatchIteratorPair GetMultiwordCompletion (libcpp_utf8_string the_key, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] meta, size_t top_n) except + # wrap-as:complete_multiword
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

        _MatchIteratorPair GetFuzzyMultiwordCompletion (libcpp_utf8_string the_key, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] meta, int32_t max_edit_distance) except + # wrap-as:complete_fuzzy_multiword
        # wrap-doc:
        #  Complete the given key to full matches after whitespace tokenizing,
        #  allowing up to max_edit_distance distance(Levenshtein).
        #  In case the used dictionary supports inner weights, the
        #  completer traverses the dictionary according to weights,
        #  otherwise byte-order.

        _MatchIteratorPair GetFuzzyMultiwordCompletion (libcpp_utf8_string the_key, libcpp_map[libcpp_utf8_string, libcpp_utf8_string] meta, int32_t max_edit_distance, size_t minimum_exact_prefix) except + # wrap-as:complete_fuzzy_multiword
        # wrap-doc:
        #  Complete the given key to full matches after whitespace tokenizing,
        #  allowing up to max_edit_distance distance(Levenshtein) except for
        #  a given exaxt prefix which must match exaxt.
        #  In case the used dictionary supports inner weights, the
        #  completer traverses the dictionary according to weights,
        #  otherwise byte-order.

        _MatchIteratorPair GetAllItems (libcpp_map[libcpp_utf8_string, libcpp_utf8_string] meta) # wrap-ignore
        libcpp_utf8_output_string GetManifest() except + # wrap-as:manifest
        libcpp_string GetStatistics() # wrap-ignore
        uint64_t GetSize() # wrap-ignore
