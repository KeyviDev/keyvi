

    def Add(self, *args):
        return call_deprecated_method("Add", "add", self.add, *args)

    def CloseFeeding(self, *args):
        return call_deprecated_method("CloseFeeding", "close_feeding", self.close_feeding, *args)

    def WriteToFile(self, *args):
        return call_deprecated_method("WriteToFile", "write_to_file", self.write_to_file, *args)
