

    def __enter__(self):
        return self


    def __setitem__(self, key, value):
        self.add(key, value)


    def __exit__(self, type, value, traceback):
        self.compile()

        
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
