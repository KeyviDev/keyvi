

    def GetAttribute(self, *args):
        """deprecated, use index based access"""
        return call_deprecated_method("GetAttribute", "[\"{key}\"]", self.__getitem__, *args)


    def __getitem__(self, key):
        if isinstance(key, unicode):
            key = key.encode("utf-8")

        py_result = self.inst.get().GetAttributePy(<libcpp_string> key)
        return <object>py_result


    def SetAttribute(self, *args):
        """deprecated, use index based access"""
        return call_deprecated_method("SetAttribute", "[\"{key}\"]", self.__setitem__, *args)


    def __setitem__(self, key, value):
        if isinstance(key, unicode):
            key = key.encode("utf-8")

        t = type(value)

        if t == bytes:
            self.inst.get().SetAttribute(<libcpp_string> key, <libcpp_string> value)
        elif t == str or t == unicode:
            self.inst.get().SetAttribute(<libcpp_string> key, <libcpp_string> value.encode('utf-8'))
        elif t == float:
            self.inst.get().SetAttribute(<libcpp_string> key, <float> value)
        elif t == int:
            self.inst.get().SetAttribute(<libcpp_string> key, <int> value)
        # special trick as t == bool does not work due to name collision between cython and C
        elif isinstance(value, (int)):
            self.inst.get().SetAttribute(<libcpp_string> key, <bool> value)
        else:
            raise Exception("Unsupported Value Type")


    @property
    def value(self):
        cdef libcpp_string packed_value = self.inst.get().GetMsgPackedValueAsString()
        if packed_value.empty():
            return None

        return msgpack.loads(packed_value)


    def GetValue(self, *args):
        """deprecated, use value property"""        
        return call_deprecated_method_getter("GetValue", "value", self.value, *args)


    def dumps(self):
        m=[]
        do_pack_rest = False
        score = self.inst.get().GetScore()
        if score != 0:
            m.append(score)
            do_pack_rest = True
        end = self.inst.get().GetEnd()
        if end != 0 or do_pack_rest:
            m.append(end)
            do_pack_rest = True
        start = self.inst.get().GetStart()
        if start != 0 or do_pack_rest:
            m.append(start)
            do_pack_rest = True
        matchedstring = self.inst.get().GetMatchedString()
        if len(matchedstring) != 0 or do_pack_rest:
            m.append(matchedstring)
            do_pack_rest = True
        rawvalue = self.inst.get().GetRawValueAsString()
        if len(rawvalue) != 0 or do_pack_rest:
            m.append(rawvalue)
        m.reverse()
        return msgpack.dumps(m)

    def __SetRawValue(self, str):
         self.inst.get().SetRawValue(<libcpp_string> str)

    @staticmethod
    def loads(serialized_match):
        m=Match()
        unserialized = msgpack.loads(serialized_match)
        number_of_fields = len(unserialized)
        if number_of_fields > 0:
            m.__SetRawValue(unserialized[0])
            if number_of_fields > 1:
                m.matched_string = unserialized[1]
                if number_of_fields > 2:
                    m.start = unserialized[2]
                    if number_of_fields > 3:
                         m.end = unserialized[3]
                         if number_of_fields > 4:
                             m.score = unserialized[4]

        return m

    @property
    def start(self):
        return self.inst.get().GetStart()

    @start.setter
    def start(self, value):
        self.inst.get().SetStart(value)

    def GetStart(self, *args):
        """deprecated, use start property"""
        return call_deprecated_method_getter("GetStart", "start", self.start, *args)

    def SetStart(self, value):
        """deprecated, use start property"""
        return call_deprecated_method_setter("SetStart", "start", lambda x : self.inst.get().SetStart(x), value)

    @property
    def end(self):
        return self.inst.get().GetEnd()

    @end.setter
    def end(self, value):
        self.inst.get().SetEnd(value)

    def GetEnd(self, *args):
        """deprecated, use end property"""
        return call_deprecated_method_getter("GetEnd", "end", self.end, *args)

    def SetEnd(self, value):
        """deprecated, use end property"""
        return call_deprecated_method_setter("SetEnd", "end", lambda x : self.inst.get().SetEnd(x), value)

    @property
    def score(self):
        return self.inst.get().GetScore()

    @score.setter
    def score(self, value):
        self.inst.get().SetScore(value)

    def GetScore(self, *args):
        """deprecated, use score property"""
        return call_deprecated_method_getter("GetScore", "score", self.score, *args)

    def SetScore(self, value):
        """deprecated, use score property"""
        return call_deprecated_method_setter("SetScore", "score", lambda x : self.inst.get().SetScore(x), value)

    @property
    def matched_string(self):
        return self.inst.get().GetMatchedString().decode('utf-8')

    @matched_string.setter
    def matched_string(self, value):
        self.inst.get().SetMatchedString(value)

    def GetMatchedString(self, *args):
        """deprecated, use matched_string property"""
        return call_deprecated_method_getter("GetMatchedString", "matched_string", self.matched_string, *args)

    def SetMatchedString(self, value):
        """deprecated, use matched_string property"""
        return call_deprecated_method_setter("SetMatchedString", "matched_string", lambda x : self.inst.get().SetMatchedString(x), value)

    def GetValueAsString(self, *args):
        """deprecated, use get_value_as_string"""
        return call_deprecated_method("GetValueAsString", "value_as_string", self.value_as_string, *args)

    def GetRawValueAsString(self, *args):
        """deprecated, use get_raw_value_as_string"""
        return call_deprecated_method("GetRawValueAsString", "dumps", self.dumps, *args)

    def raw_value_as_string(self, *args):
        """deprecated, use get_raw_value_as_string"""
        return call_deprecated_method("raw_value_as_string", "dumps", self.dumps, *args)

    def __bool__(self):
        return not self.inst.get().IsEmpty()

    def IsEmpty(self, *args):
        """deprecated, use bool operator"""
        return not call_deprecated_method("IsEmpty", "__bool__", self.__bool__, *args)

    @property
    def weight(self):
        return self.inst.get().GetWeight()
