

    def Add(self, *args):
        return call_deprecated_method("Add", "add", self.add, *args)

    def Merge(self, *args):
        return call_deprecated_method("Merge", "merge", self.merge, *args)

    def SetManifest(self, *args):
        return call_deprecated_method("SetManifest", "set_manifest", self.set_manifest, *args)
