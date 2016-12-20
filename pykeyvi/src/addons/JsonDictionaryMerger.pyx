

    def SetManifest(self, manifest):
        m = json.dumps(manifest).encode('utf-8')
        self.inst.get().SetManifestFromString(m)
