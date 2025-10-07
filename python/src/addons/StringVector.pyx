

    def Get(self, *args):
        return call_deprecated_method("Get", "__getitem__", self.__getitem__, *args)

    def Size(self, *args):
        return call_deprecated_method("Size", "__len__", self.__len__, *args)

    def Manifest(self, *args):
        return call_deprecated_method("Manifest", "manifest", self.manifest, *args)
