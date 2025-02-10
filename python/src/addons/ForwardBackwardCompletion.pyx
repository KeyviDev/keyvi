

    def GetCompletions(self, *args):
        return call_deprecated_method("GetCompletions", "complete", self.complete, *args)
