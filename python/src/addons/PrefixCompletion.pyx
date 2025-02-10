

    def GetCompletions(self, *args):
        return call_deprecated_method("GetCompletions", "complete", self.complete, *args)

    def GetFuzzyCompletions(self, *args):
        return call_deprecated_method("GetFuzzyCompletions", "complete_fuzzy", self.complete_fuzzy, *args)
