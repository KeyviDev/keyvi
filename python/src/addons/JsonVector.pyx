

    def __getitem__(self,  index ):
        assert isinstance(index, (int, int)), 'arg index wrong type'

        cdef libcpp_utf8_string _r = self.inst.get().Get((<size_t>index))
        py_result = json.loads(_r.decode('utf-8'))
        return py_result

    def Get(self, *args):
        return call_deprecated_method("Get", "__getitem__", self.__getitem__, *args)

    def Size(self, *args):
        return call_deprecated_method("Size", "__len__", self.__len__, *args)

    def Manifest(self, *args):
        return call_deprecated_method("Manifest", "manifest", self.manifest, *args)
