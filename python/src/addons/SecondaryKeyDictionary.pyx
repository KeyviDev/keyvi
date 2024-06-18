

    def get (self, the_key, meta, default = None):
        if isinstance(the_key, unicode):
            the_key = the_key.encode('utf-8')
        assert isinstance(the_key, bytes), 'arg in_0 wrong type'
        assert isinstance(meta, dict), 'arg in_1 wrong type'

        cdef libcpp_map[libcpp_utf8_string, libcpp_utf8_string] * v1 = new libcpp_map[libcpp_utf8_string, libcpp_utf8_string]()
        for _key, _value in meta.items():
            if isinstance(_key, unicode):
                _key = _key.encode('utf-8')
            if isinstance(_value, unicode):
                _value = _value.encode('utf-8')
            deref(v1)[ (<libcpp_string>_key) ] = (<libcpp_string>_value)

        cdef shared_ptr[_Match] _r = self.inst.get().GetFirst(<libcpp_string>the_key, deref(v1))
        del(v1)

        if _r.get() == nullptr:
            return default
        cdef Match py_result = Match.__new__(Match)
        py_result.inst = _r
        return py_result

    def contains(self, the_key, meta):
        if isinstance(the_key, unicode):
            the_key = the_key.encode('utf-8')

        assert isinstance(the_key, bytes), 'arg in_0 wrong type'
        assert isinstance(meta, dict), 'arg in_1 wrong type'

        return self.inst.get().Contains(the_key, meta)

    def __len__(self):
        return self.inst.get().GetSize()

    def _key_iterator_wrapper(self, iterator):
        for m in iterator:
            yield m.matched_string

    def _value_iterator_wrapper(self, iterator):
        for m in iterator:
            yield m.value

    def _item_iterator_wrapper(self, iterator):
        for m in iterator:
            yield (m.matched_string, m.value)

    def keys(self, meta):
        assert isinstance(meta, dict), 'arg in_0 wrong type'
        cdef _MatchIteratorPair _r = self.inst.get().GetAllItems(meta)
        cdef MatchIterator py_result = MatchIterator.__new__(MatchIterator)
        py_result.it = _r.begin()
        py_result.end = _r.end()
        return self._key_iterator_wrapper(py_result)

    def values(self, meta):
        assert isinstance(meta, dict), 'arg in_1 wrong type'
        cdef _MatchIteratorPair _r = self.inst.get().GetAllItems(meta)
        cdef MatchIterator py_result = MatchIterator.__new__(MatchIterator)
        py_result.it = _r.begin()
        py_result.end = _r.end()
        return self._value_iterator_wrapper(py_result)

    def items(self, meta):
        assert isinstance(meta, dict), 'arg in_1 wrong type'
        cdef _MatchIteratorPair _r = self.inst.get().GetAllItems(meta)
        cdef MatchIterator py_result = MatchIterator.__new__(MatchIterator)
        py_result.it = _r.begin()
        py_result.end = _r.end()
        return self._item_iterator_wrapper(py_result)

    def statistics(self):
        cdef libcpp_string _r = self.inst.get().GetStatistics()
        cdef bytes py_result = _r
        py_result_unicode = _r.decode('utf-8')

        return json.loads(py_result_unicode)
