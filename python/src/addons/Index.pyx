

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

        # inject keyvimerger
        if not b"keyvimerger_bin" in params:
            params[b"keyvimerger_bin"] = os.path.join(get_bin_folder(), b"keyvimerger")

        cdef libcpp_map[libcpp_utf8_string, libcpp_utf8_string] * v1 = new libcpp_map[libcpp_utf8_string, libcpp_utf8_string]()
        for key, value in params.items():
            if isinstance(key, unicode):
                key = key.encode('utf-8')
            if isinstance(value, unicode):
                value = value.encode('utf-8')
            deref(v1)[ (<libcpp_string>key) ] = (<libcpp_string>value)

        self.inst = shared_ptr[_Index](new _Index((<libcpp_string>directory), deref(v1)))
        del v1

    def __del__(self, key):
        if isinstance(key, unicode):
            key = key.encode('utf-8')

        assert isinstance(key, bytes), 'arg in_0 wrong type'

        return self.inst.get().Delete(key)