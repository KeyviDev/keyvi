

    def append(self,  in_0 ):
        dumps = json.dumps(in_0).encode('utf-8')
        self.inst.get().PushBack((<libcpp_utf8_string>dumps))

    def PushBack(self, *args):
        return call_deprecated_method("PushBack", "append", self.append, *args)

    def SetManifest(self, *args):
        return call_deprecated_method("SetManifest", "set_manifest", self.set_manifest, *args)

    def WriteToFile(self, *args):
        return call_deprecated_method("WriteToFile", "write_to_file", self.write_to_file, *args)
