#cython: c_string_encoding=ascii  # for cython>=0.19
from  libcpp.string  cimport string as libcpp_string
from  libcpp.set     cimport set as libcpp_set
from  libcpp.vector  cimport vector as libcpp_vector
from  libcpp.pair    cimport pair as libcpp_pair
from  libcpp.map     cimport map  as libcpp_map
from  smart_ptr cimport shared_ptr
from  AutowrapRefHolder cimport AutowrapRefHolder
from  libcpp cimport bool
from  libc.string cimport const_char
from cython.operator cimport dereference as deref, preincrement as inc, address as address
from cython.operator cimport dereference, preincrement
cimport cython.operator as co
from cluster cimport JumpConsistentHashString as _JumpConsistentHashString_cluster
from dictionary_compiler cimport CompletionDictionaryCompiler as _CompletionDictionaryCompiler
from dictionary cimport Dictionary as _Dictionary
from forward_backward_completion cimport ForwardBackwardCompletion as _ForwardBackwardCompletion
from predictive_compression cimport FsaPredictiveCompression as _FsaPredictiveCompression
from normalization cimport FsaTransform as _FsaTransform
from dictionary_compiler cimport JsonDictionaryCompiler as _JsonDictionaryCompiler
from dictionary_compiler cimport KeyOnlyDictionaryCompiler as _KeyOnlyDictionaryCompiler
from generator cimport KeyOnlyDictionaryGenerator as _KeyOnlyDictionaryGenerator
from match cimport Match as _Match
from match_iterator cimport MatchIterator as _MatchIterator
from match_iterator cimport MatchIteratorPair as _MatchIteratorPair
from multi_word_completion cimport MultiWordCompletion as _MultiWordCompletion
from prefix_completion cimport PrefixCompletion as _PrefixCompletion
from dictionary_compiler cimport StringDictionaryCompiler as _StringDictionaryCompiler
cdef extern from "autowrap_tools.hpp":
    char * _cast_const_away(char *)

def JumpConsistentHashString(bytes in_0 ,  in_1 ):
    assert isinstance(in_0, bytes), 'arg in_0 wrong type'
    assert isinstance(in_1, (int, long)), 'arg in_1 wrong type'
    cdef const_char * input_in_0 = <const_char *> in_0

    cdef uint32_t _r = _JumpConsistentHashString_cluster(input_in_0, (<uint32_t>in_1))
    py_result = <uint32_t>_r
    return py_result 

cdef class StringDictionaryCompiler:

    cdef shared_ptr[_StringDictionaryCompiler] inst

    def __dealloc__(self):
         self.inst.reset()

    
    def _init_0(self):
        self.inst = shared_ptr[_StringDictionaryCompiler](new _StringDictionaryCompiler())
    
    def _init_1(self,  memory_limit ):
        assert isinstance(memory_limit, (int, long)), 'arg memory_limit wrong type'
    
        self.inst = shared_ptr[_StringDictionaryCompiler](new _StringDictionaryCompiler((<size_t>memory_limit)))
    
    def __init__(self, *args):
        if not args:
             self._init_0(*args)
        elif (len(args)==1) and (isinstance(args[0], (int, long))):
             self._init_1(*args)
        else:
               raise Exception('can not handle type of %s' % (args,))
    
    def Add(self, bytes in_0 , bytes in_1 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
        assert isinstance(in_1, bytes), 'arg in_1 wrong type'
        cdef const_char * input_in_0 = <const_char *> in_0
        cdef const_char * input_in_1 = <const_char *> in_1
        self.inst.get().Add(input_in_0, input_in_1)
    
    def WriteToFile(self, bytes in_0 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
        cdef const_char * input_in_0 = <const_char *> in_0
        self.inst.get().WriteToFile(input_in_0)
    
    def __setitem__(self, bytes in_0 , bytes in_1 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
        assert isinstance(in_1, bytes), 'arg in_1 wrong type'
        cdef const_char * input_in_0 = <const_char *> in_0
        cdef const_char * input_in_1 = <const_char *> in_1
        self.inst.get().__setitem__(input_in_0, input_in_1)
    
    def SetManifestFromString(self, bytes in_0 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
        cdef const_char * input_in_0 = <const_char *> in_0
        self.inst.get().SetManifestFromString(input_in_0)
    
    def __enter__(self):
        return self

    
    def __exit__(self, type, value, traceback):
        self.Compile()


    def Compile(self, *args):
        if not args:
            with nogil:
                self.inst.get().Compile()
            return

        cdef void* callback = <void*> args[0]
        with nogil:
            self.inst.get().Compile(callback_wrapper, callback) 

cdef class JsonDictionaryCompiler:

    cdef shared_ptr[_JsonDictionaryCompiler] inst

    def __dealloc__(self):
         self.inst.reset()

    
    def Add(self, bytes in_0 , bytes in_1 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
        assert isinstance(in_1, bytes), 'arg in_1 wrong type'
        cdef const_char * input_in_0 = <const_char *> in_0
        cdef const_char * input_in_1 = <const_char *> in_1
        self.inst.get().Add(input_in_0, input_in_1)
    
    def _init_0(self):
        self.inst = shared_ptr[_JsonDictionaryCompiler](new _JsonDictionaryCompiler())
    
    def _init_1(self,  memory_limit ):
        assert isinstance(memory_limit, (int, long)), 'arg memory_limit wrong type'
    
        self.inst = shared_ptr[_JsonDictionaryCompiler](new _JsonDictionaryCompiler((<size_t>memory_limit)))
    
    def __init__(self, *args):
        if not args:
             self._init_0(*args)
        elif (len(args)==1) and (isinstance(args[0], (int, long))):
             self._init_1(*args)
        else:
               raise Exception('can not handle type of %s' % (args,))
    
    def WriteToFile(self, bytes in_0 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
        cdef const_char * input_in_0 = <const_char *> in_0
        self.inst.get().WriteToFile(input_in_0)
    
    def __setitem__(self, bytes in_0 , bytes in_1 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
        assert isinstance(in_1, bytes), 'arg in_1 wrong type'
        cdef const_char * input_in_0 = <const_char *> in_0
        cdef const_char * input_in_1 = <const_char *> in_1
        self.inst.get().__setitem__(input_in_0, input_in_1)
    
    def SetManifestFromString(self, bytes in_0 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
        cdef const_char * input_in_0 = <const_char *> in_0
        self.inst.get().SetManifestFromString(input_in_0)
    
    def __enter__(self):
        return self

    
    def __exit__(self, type, value, traceback):
        self.Compile()

        
    def Compile(self, *args):
        if not args:
            with nogil:
                self.inst.get().Compile()
            return

        cdef void* callback = <void*> args[0]
        with nogil:
            self.inst.get().Compile(callback_wrapper, callback) 

cdef class Dictionary:

    cdef shared_ptr[_Dictionary] inst

    def __dealloc__(self):
         self.inst.reset()

    
    def GetManifestAsString(self):
        cdef libcpp_string _r = self.inst.get().GetManifestAsString()
        py_result = <libcpp_string>_r
        return py_result
    
    def Lookup(self, bytes in_0 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
        cdef const_char * input_in_0 = <const_char *> in_0
        cdef _MatchIteratorPair _r = self.inst.get().Lookup(input_in_0)
        cdef MatchIterator py_result = MatchIterator.__new__(MatchIterator)
        py_result.it = _r.begin()
        py_result.end = _r.end()
        return py_result
    
    def LookupText(self, bytes in_0 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
        cdef const_char * input_in_0 = <const_char *> in_0
        cdef _MatchIteratorPair _r = self.inst.get().LookupText(input_in_0)
        cdef MatchIterator py_result = MatchIterator.__new__(MatchIterator)
        py_result.it = _r.begin()
        py_result.end = _r.end()
        return py_result
    
    def __init__(self, bytes filename ):
        assert isinstance(filename, bytes), 'arg filename wrong type'
        cdef const_char * input_filename = <const_char *> filename
        self.inst = shared_ptr[_Dictionary](new _Dictionary(input_filename))
    
    def Get(self, bytes in_0 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
        cdef const_char * input_in_0 = <const_char *> in_0
        cdef _MatchIteratorPair _r = self.inst.get().Get(input_in_0)
        cdef MatchIterator py_result = MatchIterator.__new__(MatchIterator)
        py_result.it = _r.begin()
        py_result.end = _r.end()
        return py_result
    
    def get (self, key, default = None):
        assert isinstance(key, bytes), 'arg in_0 wrong type'
    
        cdef _Match * _r = new _Match(deref(self.inst.get())[(<const_char *>key)])

        if _r.IsEmpty():
            return default
        cdef Match py_result = Match.__new__(Match)
        py_result.inst = shared_ptr[_Match](_r)
        return py_result

    def __contains__(self, key):
        assert isinstance(key, bytes), 'arg in_0 wrong type'

        return self.inst.get().Contains(key)

    def __len__(self):
        return self.inst.get().GetSize()

    def __getitem__ (self, key):
        assert isinstance(key, bytes), 'arg in_0 wrong type'
    
        cdef _Match * _r = new _Match(deref(self.inst.get())[(<const_char *>key)])

        if _r.IsEmpty():
            raise KeyError(key)
        cdef Match py_result = Match.__new__(Match)
        py_result.inst = shared_ptr[_Match](_r)
        return py_result

    def _key_iterator_wrapper(self, iterator):
        for m in iterator:
            yield m.GetMatchedString()

    def _value_iterator_wrapper(self, iterator):
        for m in iterator:
            yield m.GetRawValueAsString()

    def _item_iterator_wrapper(self, iterator):
        for m in iterator:
            yield (m.GetMatchedString(), m.GetRawValueAsString())

    def GetAllKeys(self):
        cdef _MatchIteratorPair _r = self.inst.get().GetAllItems()
        cdef MatchIterator py_result = MatchIterator.__new__(MatchIterator)
        py_result.it = _r.begin()
        py_result.end = _r.end()
        return self._key_iterator_wrapper(py_result)

    def GetAllValues(self):
        cdef _MatchIteratorPair _r = self.inst.get().GetAllItems()
        cdef MatchIterator py_result = MatchIterator.__new__(MatchIterator)
        py_result.it = _r.begin()
        py_result.end = _r.end()
        return self._value_iterator_wrapper(py_result)

    def GetAllItems(self):
        cdef _MatchIteratorPair _r = self.inst.get().GetAllItems()
        cdef MatchIterator py_result = MatchIterator.__new__(MatchIterator)
        py_result.it = _r.begin()
        py_result.end = _r.end()
        return self._item_iterator_wrapper(py_result)

    def GetStatistics(self):
        cdef libcpp_string _r = self.inst.get().GetStatistics()
        py_result = <libcpp_string>_r
        import json
        return {k: json.loads(v) for k, v in filter(
            lambda kv: kv[1], [s.split("\n") for s in py_result.split("\n\n")])} 

cdef class FsaTransform:

    cdef shared_ptr[_FsaTransform] inst

    def __dealloc__(self):
         self.inst.reset()

    
    def Normalize(self, bytes in_0 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
    
        cdef libcpp_string _r = self.inst.get().Normalize((<libcpp_string>in_0))
        py_result = <libcpp_string>_r
        return py_result
    
    def __init__(self, Dictionary in_0 ):
        assert isinstance(in_0, Dictionary), 'arg in_0 wrong type'
        cdef shared_ptr[_Dictionary] input_in_0 = in_0.inst
        self.inst = shared_ptr[_FsaTransform](new _FsaTransform(input_in_0)) 

cdef class PrefixCompletion:

    cdef shared_ptr[_PrefixCompletion] inst

    def __dealloc__(self):
         self.inst.reset()

    
    def GetFuzzyCompletions(self, bytes in_0 ,  max_edit_distance ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
        assert isinstance(max_edit_distance, (int, long)), 'arg max_edit_distance wrong type'
        cdef const_char * input_in_0 = <const_char *> in_0
    
        cdef _MatchIteratorPair _r = self.inst.get().GetFuzzyCompletions(input_in_0, (<int>max_edit_distance))
        cdef MatchIterator py_result = MatchIterator.__new__(MatchIterator)
        py_result.it = _r.begin()
        py_result.end = _r.end()
        return py_result
    
    def __init__(self, Dictionary in_0 ):
        assert isinstance(in_0, Dictionary), 'arg in_0 wrong type'
        cdef shared_ptr[_Dictionary] input_in_0 = in_0.inst
        self.inst = shared_ptr[_PrefixCompletion](new _PrefixCompletion(input_in_0))
    
    def GetCompletions(self, bytes in_0 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
        cdef const_char * input_in_0 = <const_char *> in_0
        cdef _MatchIteratorPair _r = self.inst.get().GetCompletions(input_in_0)
        cdef MatchIterator py_result = MatchIterator.__new__(MatchIterator)
        py_result.it = _r.begin()
        py_result.end = _r.end()
        return py_result 

cdef class ForwardBackwardCompletion:

    cdef shared_ptr[_ForwardBackwardCompletion] inst

    def __dealloc__(self):
         self.inst.reset()

    
    def _GetCompletions_0(self, bytes in_0 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
        cdef const_char * input_in_0 = <const_char *> in_0
        cdef _MatchIteratorPair _r = self.inst.get().GetCompletions(input_in_0)
        cdef MatchIterator py_result = MatchIterator.__new__(MatchIterator)
        py_result.it = _r.begin()
        py_result.end = _r.end()
        return py_result
    
    def _GetCompletions_1(self, bytes in_0 ,  in_1 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
        assert isinstance(in_1, (int, long)), 'arg in_1 wrong type'
        cdef const_char * input_in_0 = <const_char *> in_0
    
        cdef _MatchIteratorPair _r = self.inst.get().GetCompletions(input_in_0, (<int>in_1))
        cdef MatchIterator py_result = MatchIterator.__new__(MatchIterator)
        py_result.it = _r.begin()
        py_result.end = _r.end()
        return py_result
    
    def GetCompletions(self, *args):
        if (len(args)==1) and (isinstance(args[0], bytes)):
            return self._GetCompletions_0(*args)
        elif (len(args)==2) and (isinstance(args[0], bytes)) and (isinstance(args[1], (int, long))):
            return self._GetCompletions_1(*args)
        else:
               raise Exception('can not handle type of %s' % (args,))
    
    def __init__(self, Dictionary in_0 , Dictionary in_1 ):
        assert isinstance(in_0, Dictionary), 'arg in_0 wrong type'
        assert isinstance(in_1, Dictionary), 'arg in_1 wrong type'
        cdef shared_ptr[_Dictionary] input_in_0 = in_0.inst
        cdef shared_ptr[_Dictionary] input_in_1 = in_1.inst
        self.inst = shared_ptr[_ForwardBackwardCompletion](new _ForwardBackwardCompletion(input_in_0, input_in_1)) 

cdef class FsaPredictiveCompression:

    cdef shared_ptr[_FsaPredictiveCompression] inst

    def __dealloc__(self):
         self.inst.reset()

    
    def __init__(self, Dictionary in_0 ):
        assert isinstance(in_0, Dictionary), 'arg in_0 wrong type'
        cdef shared_ptr[_Dictionary] input_in_0 = in_0.inst
        self.inst = shared_ptr[_FsaPredictiveCompression](new _FsaPredictiveCompression(input_in_0))
    
    def Compress(self, bytes in_0 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
    
        cdef libcpp_string _r = self.inst.get().Compress((<libcpp_string>in_0))
        py_result = <libcpp_string>_r
        return py_result
    
    def Uncompress(self, bytes in_0 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
    
        cdef libcpp_string _r = self.inst.get().Uncompress((<libcpp_string>in_0))
        py_result = <libcpp_string>_r
        return py_result 

cdef class CompletionDictionaryCompiler:

    cdef shared_ptr[_CompletionDictionaryCompiler] inst

    def __dealloc__(self):
         self.inst.reset()

    
    def __setitem__(self, bytes in_0 ,  in_1 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
        assert isinstance(in_1, (int, long)), 'arg in_1 wrong type'
        cdef const_char * input_in_0 = <const_char *> in_0
    
        self.inst.get().__setitem__(input_in_0, (<int>in_1))
    
    def Add(self, bytes in_0 ,  in_1 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
        assert isinstance(in_1, (int, long)), 'arg in_1 wrong type'
        cdef const_char * input_in_0 = <const_char *> in_0
    
        self.inst.get().Add(input_in_0, (<int>in_1))
    
    def WriteToFile(self, bytes in_0 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
        cdef const_char * input_in_0 = <const_char *> in_0
        self.inst.get().WriteToFile(input_in_0)
    
    def _init_0(self):
        self.inst = shared_ptr[_CompletionDictionaryCompiler](new _CompletionDictionaryCompiler())
    
    def _init_1(self,  memory_limit ):
        assert isinstance(memory_limit, (int, long)), 'arg memory_limit wrong type'
    
        self.inst = shared_ptr[_CompletionDictionaryCompiler](new _CompletionDictionaryCompiler((<size_t>memory_limit)))
    
    def __init__(self, *args):
        if not args:
             self._init_0(*args)
        elif (len(args)==1) and (isinstance(args[0], (int, long))):
             self._init_1(*args)
        else:
               raise Exception('can not handle type of %s' % (args,))
    
    def SetManifestFromString(self, bytes in_0 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
        cdef const_char * input_in_0 = <const_char *> in_0
        self.inst.get().SetManifestFromString(input_in_0)
    
    def __enter__(self):
        return self

    
    def __exit__(self, type, value, traceback):
        self.Compile()

        
    def Compile(self, *args):
        if not args:
            with nogil:
                self.inst.get().Compile()
            return

        cdef void* callback = <void*> args[0]
        with nogil:
            self.inst.get().Compile(callback_wrapper, callback)

# definition for all compilers
cdef void callback_wrapper(size_t a, size_t b, void* py_callback) with gil:
    (<object>py_callback)(a, b) 

cdef class MultiWordCompletion:

    cdef shared_ptr[_MultiWordCompletion] inst

    def __dealloc__(self):
         self.inst.reset()

    
    def __init__(self, Dictionary in_0 ):
        assert isinstance(in_0, Dictionary), 'arg in_0 wrong type'
        cdef shared_ptr[_Dictionary] input_in_0 = in_0.inst
        self.inst = shared_ptr[_MultiWordCompletion](new _MultiWordCompletion(input_in_0))
    
    def _GetCompletions_0(self, bytes in_0 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
        cdef const_char * input_in_0 = <const_char *> in_0
        cdef _MatchIteratorPair _r = self.inst.get().GetCompletions(input_in_0)
        cdef MatchIterator py_result = MatchIterator.__new__(MatchIterator)
        py_result.it = _r.begin()
        py_result.end = _r.end()
        return py_result
    
    def _GetCompletions_1(self, bytes in_0 ,  in_1 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
        assert isinstance(in_1, (int, long)), 'arg in_1 wrong type'
        cdef const_char * input_in_0 = <const_char *> in_0
    
        cdef _MatchIteratorPair _r = self.inst.get().GetCompletions(input_in_0, (<int>in_1))
        cdef MatchIterator py_result = MatchIterator.__new__(MatchIterator)
        py_result.it = _r.begin()
        py_result.end = _r.end()
        return py_result
    
    def GetCompletions(self, *args):
        if (len(args)==1) and (isinstance(args[0], bytes)):
            return self._GetCompletions_0(*args)
        elif (len(args)==2) and (isinstance(args[0], bytes)) and (isinstance(args[1], (int, long))):
            return self._GetCompletions_1(*args)
        else:
               raise Exception('can not handle type of %s' % (args,)) 

cdef class KeyOnlyDictionaryGenerator:

    cdef shared_ptr[_KeyOnlyDictionaryGenerator] inst

    def __dealloc__(self):
         self.inst.reset()

    
    def __init__(self):
        self.inst = shared_ptr[_KeyOnlyDictionaryGenerator](new _KeyOnlyDictionaryGenerator())
    
    def Add(self, bytes in_0 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
        cdef const_char * input_in_0 = <const_char *> in_0
        self.inst.get().Add(input_in_0)
    
    def CloseFeeding(self):
        self.inst.get().CloseFeeding()
    
    def WriteToFile(self, bytes in_0 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
        cdef const_char * input_in_0 = <const_char *> in_0
        self.inst.get().WriteToFile(input_in_0) 

cdef class KeyOnlyDictionaryCompiler:

    cdef shared_ptr[_KeyOnlyDictionaryCompiler] inst

    def __dealloc__(self):
         self.inst.reset()

    
    def _init_0(self):
        self.inst = shared_ptr[_KeyOnlyDictionaryCompiler](new _KeyOnlyDictionaryCompiler())
    
    def _init_1(self,  memory_limit ):
        assert isinstance(memory_limit, (int, long)), 'arg memory_limit wrong type'
    
        self.inst = shared_ptr[_KeyOnlyDictionaryCompiler](new _KeyOnlyDictionaryCompiler((<size_t>memory_limit)))
    
    def __init__(self, *args):
        if not args:
             self._init_0(*args)
        elif (len(args)==1) and (isinstance(args[0], (int, long))):
             self._init_1(*args)
        else:
               raise Exception('can not handle type of %s' % (args,))
    
    def Add(self, bytes in_0 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
        cdef const_char * input_in_0 = <const_char *> in_0
        self.inst.get().Add(input_in_0)
    
    def SetManifestFromString(self, bytes in_0 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
        cdef const_char * input_in_0 = <const_char *> in_0
        self.inst.get().SetManifestFromString(input_in_0)
    
    def WriteToFile(self, bytes in_0 ):
        assert isinstance(in_0, bytes), 'arg in_0 wrong type'
        cdef const_char * input_in_0 = <const_char *> in_0
        self.inst.get().WriteToFile(input_in_0)
    
    def __enter__(self):
        return self

    
    def __exit__(self, type, value, traceback):
        self.Compile()


    def Compile(self, *args):
        if not args:
            with nogil:
                self.inst.get().Compile()
            return

        cdef void* callback = <void*> args[0]
        with nogil:
            self.inst.get().Compile(callback_wrapper, callback) 

cdef class Match:

    cdef shared_ptr[_Match] inst

    def __dealloc__(self):
         self.inst.reset()

    
    def SetEnd(self,  end ):
        assert isinstance(end, (int, long)), 'arg end wrong type'
    
        self.inst.get().SetEnd((<size_t>end))
    
    def GetStart(self):
        cdef size_t _r = self.inst.get().GetStart()
        py_result = <size_t>_r
        return py_result
    
    def GetScore(self):
        cdef float _r = self.inst.get().GetScore()
        py_result = <float>_r
        return py_result
    
    def SetMatchedString(self, bytes matched_string ):
        assert isinstance(matched_string, bytes), 'arg matched_string wrong type'
    
        self.inst.get().SetMatchedString((<libcpp_string>matched_string))
    
    def GetValueAsString(self):
        cdef libcpp_string _r = self.inst.get().GetValueAsString()
        py_result = <libcpp_string>_r
        return py_result
    
    def IsEmpty(self):
        cdef bool _r = self.inst.get().IsEmpty()
        py_result = <bool>_r
        return py_result
    
    def SetScore(self, float score ):
        assert isinstance(score, float), 'arg score wrong type'
    
        self.inst.get().SetScore((<float>score))
    
    def GetRawValueAsString(self):
        cdef libcpp_string _r = self.inst.get().GetRawValueAsString()
        py_result = <libcpp_string>_r
        return py_result
    
    def SetStart(self,  start ):
        assert isinstance(start, (int, long)), 'arg start wrong type'
    
        self.inst.get().SetStart((<size_t>start))
    
    def GetEnd(self):
        cdef size_t _r = self.inst.get().GetEnd()
        py_result = <size_t>_r
        return py_result
    
    def __copy__(self):
       cdef Match rv = Match.__new__(Match)
       rv.inst = shared_ptr[_Match](new _Match(deref(self.inst.get())))
       return rv
    
    def __deepcopy__(self, memo):
       cdef Match rv = Match.__new__(Match)
       rv.inst = shared_ptr[_Match](new _Match(deref(self.inst.get())))
       return rv
    
    def _init_0(self):
        self.inst = shared_ptr[_Match](new _Match())
    
    def _init_1(self, Match m ):
        assert isinstance(m, Match), 'arg m wrong type'
    
        self.inst = shared_ptr[_Match](new _Match((deref(m.inst.get()))))
    
    def __init__(self, *args):
        if not args:
             self._init_0(*args)
        elif (len(args)==1) and (isinstance(args[0], Match)):
             self._init_1(*args)
        else:
               raise Exception('can not handle type of %s' % (args,))
    
    def GetMatchedString(self):
        cdef libcpp_string _r = self.inst.get().GetMatchedString()
        py_result = <libcpp_string>_r
        return py_result
    
    def GetAttribute(self, key):
        py_result = self.inst.get().GetAttributePy(<libcpp_string> key)
        return <object>py_result
        
        
    def SetAttribute(self, key, value):
        t = type(value)
        if t == str:
            self.inst.get().SetAttribute(<libcpp_string> key, <libcpp_string> value)
        elif t == float:
            self.inst.get().SetAttribute(<libcpp_string> key, <float> value)
        elif t == int:
            self.inst.get().SetAttribute(<libcpp_string> key, <int> value)
        # special trick as t == bool does not work due to name collision between cython and C
        elif isinstance(value, (int)):
            self.inst.get().SetAttribute(<libcpp_string> key, <bool> value)
        else:
            raise Exception("Unsupported Value Type") 
 
 
 
# import uint32_t type
from libc.stdint cimport uint32_t 
 
# same import style as autowrap
from match cimport Match as _Match
from match_iterator cimport MatchIterator as _MatchIterator

cdef class MatchIterator:
    cdef _MatchIterator it
    cdef _MatchIterator end

    #def __cinit__(self, ):
    #    self.end = new _MatchIterator()

    # Most likely, you will be calling this directly from this
    # or another Cython module, not from Python.
    #cdef set_iter(self, _MatchIterator it):
    #    self.it = it

    def __iter__(self):
        return self

    #def __dealloc__(self):
        # This works by calling "delete" in C++, you should not
        # fear that Cython will call "free"
    #    del self.it
    #    del self.end

    def __next__(self):
        # This works correctly by using "*it" and "*end" in the code,
        #if  co.dereference( self.it ) == co.dereference( self.end ) :
        if  self.it == self.end:

            raise StopIteration()
        cdef _Match * _r = new _Match(co.dereference( self.it ))

        # This also does the expected thing.
        co.preincrement( self.it )

        cdef Match py_result = Match.__new__(Match)
        py_result.inst = shared_ptr[_Match](_r)

        return py_result 
 
 
