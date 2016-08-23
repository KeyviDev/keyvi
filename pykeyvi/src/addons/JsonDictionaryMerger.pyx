

    def SetManifest(self, manifest):
        m = json.dumps(manifest)
        self.inst.get().SetManifestFromString(m)
