

    def __enter__(self):
        return self


    def __setitem__(self, key, value):
        self.Add(key, value)


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

