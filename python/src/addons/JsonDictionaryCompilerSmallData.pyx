

    def __enter__(self):
        return self


    def __setitem__(self, key, value):
        self.Add(key, value)


    def __exit__(self, type, value, traceback):
        self.Compile()


    def Add(self, key , value ):
        assert isinstance(key, (bytes, unicode)), 'arg in_0 wrong type'
        assert isinstance(value, (bytes, unicode)), 'arg in_1 wrong type'

        if isinstance(key, unicode):
            key = key.encode('UTF-8')
        cdef libcpp_string input_in_0 = <libcpp_string> key

        if isinstance(value, unicode):
            value = value.encode('UTF-8')
        cdef libcpp_string input_in_1 = <libcpp_string> value

        self.inst.get().Add(input_in_0, input_in_1)

        
    def Compile(self, *args):
        if not args:
            with nogil:
                self.inst.get().Compile()
            return

        cdef void* callback = <void*> args[0]
        with nogil:
            self.inst.get().Compile(callback_wrapper, callback)

