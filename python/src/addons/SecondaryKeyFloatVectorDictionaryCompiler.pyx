

    def __enter__(self):
        return self


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

