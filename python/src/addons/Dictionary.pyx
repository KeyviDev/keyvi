

    def get (self, key, default = None):
        if isinstance(key, unicode):
            key = key.encode('utf-8')
        assert isinstance(key, bytes), 'arg in_0 wrong type'
    
        cdef shared_ptr[_Match] _r = deref(self.inst.get())[(<libcpp_string>key)]

        if _r.get() == nullptr:
            return default
        cdef Match py_result = Match.__new__(Match)
        py_result.inst = _r
        return py_result

    def __contains__(self, key):
        if isinstance(key, unicode):
            key = key.encode('utf-8')

        assert isinstance(key, bytes), 'arg in_0 wrong type'

        return self.inst.get().Contains(key)

    def __len__(self):
        return self.inst.get().GetSize()

    def __getitem__ (self, key):
        if isinstance(key, unicode):
            key = key.encode('utf-8')

        assert isinstance(key, bytes), 'arg in_0 wrong type'
    
        cdef shared_ptr[_Match] _r = deref(self.inst.get())[(<libcpp_string>key)]

        if _r.get() == nullptr:
            raise KeyError(key)
        cdef Match py_result = Match.__new__(Match)
        py_result.inst = _r
        return py_result

    def _key_iterator_wrapper(self, iterator):
        for m in iterator:
            yield m.matched_string

    def _value_iterator_wrapper(self, iterator):
        for m in iterator:
            yield m.value

    def _item_iterator_wrapper(self, iterator):
        for m in iterator:
            yield (m.matched_string, m.value)

    def keys(self):
        cdef _MatchIteratorPair _r = self.inst.get().GetAllItems()
        cdef MatchIterator py_result = MatchIterator.__new__(MatchIterator)
        py_result.it = _r.begin()
        py_result.end = _r.end()
        return self._key_iterator_wrapper(py_result)

    def GetAllKeys(self):
        return call_deprecated_method("GetAllKeys", "keys", self.keys)

    def values(self):
        cdef _MatchIteratorPair _r = self.inst.get().GetAllItems()
        cdef MatchIterator py_result = MatchIterator.__new__(MatchIterator)
        py_result.it = _r.begin()
        py_result.end = _r.end()
        return self._value_iterator_wrapper(py_result)

    def GetAllValues(self):
        return call_deprecated_method("GetAllValues", "values", self.values)

    def items(self):
        cdef _MatchIteratorPair _r = self.inst.get().GetAllItems()
        cdef MatchIterator py_result = MatchIterator.__new__(MatchIterator)
        py_result.it = _r.begin()
        py_result.end = _r.end()
        return self._item_iterator_wrapper(py_result)

    def GetAllItems(self):
        return call_deprecated_method("GetAllItems", "items", self.items)

    def statistics(self):
        cdef libcpp_string _r = self.inst.get().GetStatistics()
        cdef bytes py_result = _r
        py_result_unicode = _r.decode('utf-8')

        return json.loads(py_result_unicode)

    def GetStatistics(self):
        return call_deprecated_method("GetStatistics", "statistics", self.statistics)

    def GetNear(self, *args):
        return call_deprecated_method("GetNear", "match_near", self.match_near, *args)

    def Get(self, *args):
        return call_deprecated_method("Get", "match", self.match, *args)

    def GetFuzzy(self, *args):
        return call_deprecated_method("GetFuzzy", "match_fuzzy", self.match_fuzzy, *args)

    def Lookup(self, *args):
        return call_deprecated_method("Lookup", "search", self.search, *args)

    def LookupText(self, *args):
        return call_deprecated_method("LookupText", "search_tokenized", self.search_tokenized, *args)

    def GetManifest(self, *args):
        return call_deprecated_method("GetManifest", "manifest", self.manifest, *args)

