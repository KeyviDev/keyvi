

    def get (self, key, default = None):
        """Return the value for key if key is in the dictionary, else default."""
        if isinstance(key, unicode):
            key = key.encode('utf-8')
        assert isinstance(key, bytes), 'arg in_0 wrong type'

        cdef shared_ptr[_Match] _r = deref(self.inst.get())[(<libcpp_string>key)]

        if _r.get() == nullptr:
            return default
        cdef Match py_result = Match.__new__(Match)
        py_result.inst = _r
        return py_result

    def Get(self, *args):
        return call_deprecated_method("Get", "get", self.get, *args)

    def __contains__(self, key):
        if isinstance(key, unicode):
            key = key.encode('utf-8')

        assert isinstance(key, bytes), 'arg in_0 wrong type'

        return self.inst.get().Contains(key)

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

    def GetNear(self, *args):
        return call_deprecated_method("GetNear", "get_near", self.get_near, *args)

    def GetFuzzy(self, *args):
        return call_deprecated_method("GetFuzzy", "get_fuzzy", self.get_fuzzy, *args)
