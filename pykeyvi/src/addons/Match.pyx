

    def GetAttribute(self, key):
        py_result = self.inst.get().GetAttributePy(<libcpp_string> key)
        return <object>py_result
        
        
    def SetAttribute(self, key, value):
        t = type(value)
        if t == str:
            self.inst.get().SetAttribute(<libcpp_string> key, <libcpp_string> value)
        elif t == float:
            self.inst.get().SetAttribute(<libcpp_string> key, <float> value)
        elif t == int:
            self.inst.get().SetAttribute(<libcpp_string> key, <int> value)
        # special trick as t == bool does not work due to name collision between cython and C
        elif isinstance(value, (int)):
            self.inst.get().SetAttribute(<libcpp_string> key, <bool> value)
        else:
            raise Exception("Unsupported Value Type")


from match cimport EncodeJsonValue as _EncodeJsonValue 


def EncodeJsonValue(libcpp_string raw_value, compression_threshold=None):
    if compression_threshold:
        return _EncodeJsonValue(raw_value, compression_threshold)
    else:
        return _EncodeJsonValue(raw_value)

