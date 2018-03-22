

    def PushBack(self,  in_0 ):
        dumps = json.dumps(in_0).encode('utf-8')
        self.inst.get().PushBack((<libcpp_utf8_string>dumps))
