

    def __enter__(self):
        return self


    def __setitem__(self, key, value):
        self.add(key, value)


    def __exit__(self, type, value, traceback):
        self.compile()


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

        
    def compile(self, *args):
        if not args:
            with nogil:
                self.inst.get().Compile()
            return

        cdef void* callback = <void*> args[0]
        with nogil:
            self.inst.get().Compile(progress_compiler_callback, callback)

    def Compile(self, *args):
        return call_deprecated_method("Compile", "compile", self.compile, *args)

    def Add(self, *args):
        return call_deprecated_method("Add", "add", self.add, *args)

    def SetManifest(self, *args):
        return call_deprecated_method("SetManifest", "set_manifest", self.set_manifest, *args)

    def WriteToFile(self, *args):
        return call_deprecated_method("WriteToFile", "write_to_file", self.write_to_file, *args)
