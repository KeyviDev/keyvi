

    def __init__(self, *args , **kwargs):
        if (len(args)==1) and (isinstance(args[0], (bytes, unicode))):
            directory = args[0]
            params = {}
        elif (len(args)==2) and (isinstance(args[0], (bytes, unicode))) and (isinstance(args[1], dict) and all(isinstance(k, (bytes, unicode)) for k in args[1].keys()) and all(isinstance(v, (bytes, unicode)) for v in args[1].values())):
            directory = args[0]
            params = args[1]
        else:
            raise Exception('can not handle type of %s' % (args,))

        if isinstance(directory, unicode):
            directory = directory.encode('UTF-8')

        # inject a keyvimerger shipped with the package if not set from outside
        if not b"keyvimerger_bin" in params:
            params[b"keyvimerger_bin"] = get_interpreter_executable() + b" " + os.path.join(get_package_root(), b"_pycore" , b"keyvimerger.py")

        cdef libcpp_map[libcpp_utf8_string, libcpp_utf8_string] * v1 = new libcpp_map[libcpp_utf8_string, libcpp_utf8_string]()
        for key, value in params.items():
            if isinstance(key, unicode):
                key = key.encode('utf-8')
            if isinstance(value, unicode):
                value = value.encode('utf-8')
            deref(v1)[ (<libcpp_string>key) ] = (<libcpp_string>value)

        self.inst = shared_ptr[_Index](new _Index((<libcpp_string>directory), deref(v1)))
        del v1

    def __delitem__(self, key):
        if isinstance(key, unicode):
            key = key.encode('utf-8')

        assert isinstance(key, bytes), 'arg in_0 wrong type'

        self.inst.get().Delete(key)

    def Get (self, key, default = None):
        """Return the value for key if key is in the dictionary, else default."""
        if isinstance(key, unicode):
            key = key.encode('utf-8')
        assert isinstance(key, bytes), 'arg in_0 wrong type'

        cdef shared_ptr[_Match] _r = shared_ptr[_Match](new _Match(deref(self.inst.get())[(<libcpp_string>key)]))

        if _r.get().IsEmpty():
            return default
        cdef Match py_result = Match.__new__(Match)
        py_result.inst = _r
        return py_result

    def __contains__(self, key):
        if isinstance(key, unicode):
            key = key.encode('utf-8')

        assert isinstance(key, bytes), 'arg in_0 wrong type'

        return self.inst.get().Contains(key)

    def __getitem__ (self, key):
        if isinstance(key, unicode):
            key = key.encode('utf-8')

        assert isinstance(key, bytes), 'arg in_0 wrong type'

        cdef shared_ptr[_Match] _r = shared_ptr[_Match](new _Match(deref(self.inst.get())[(<libcpp_string>key)]))

        if _r.get().IsEmpty():
            raise KeyError(key)
        cdef Match py_result = Match.__new__(Match)
        py_result.inst = _r
        return py_result

    def MSet(self, list key_values ):
        assert isinstance(key_values, list), 'arg in_0 wrong type'
        cdef s_shared_ptr[libcpp_vector[libcpp_pair[libcpp_utf8_string,libcpp_utf8_string]]] cpp_key_values = s_shared_ptr[libcpp_vector[libcpp_pair[libcpp_utf8_string,libcpp_utf8_string]]](new libcpp_vector[libcpp_pair[libcpp_utf8_string,libcpp_utf8_string]]())
        cdef libcpp_pair[libcpp_utf8_string, libcpp_utf8_string] cpp_kv

        for kv in key_values:
            assert isinstance (kv, tuple), 'arg in_0 wrong type'
            assert isinstance(kv[0], (bytes, unicode)), 'arg in_0 wrong type'
            assert isinstance(kv[1], (bytes, unicode)), 'arg in_1 wrong type'
            key = kv[0]
            if isinstance(key, unicode):
                key = key.encode('utf-8')
            value = kv[1]
            if isinstance(value, unicode):
                value = value.encode('utf-8')
            cpp_kv.first = key
            cpp_kv.second = value
            cpp_key_values.get().push_back(cpp_kv)

        self.inst.get().MSet(cpp_key_values)
