

    def Get(self,  index ):
        assert isinstance(index, (int, long)), 'arg index wrong type'

        cdef libcpp_utf8_string _r = self.inst.get().Get((<size_t>index))
        py_result = json.loads(_r.decode('utf-8'))
        return py_result
