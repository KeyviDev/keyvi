# -*- coding: utf-8 -*-
# some common tools for tests

import contextlib
import os
import pykeyvi

@contextlib.contextmanager
def tmp_dictionary(compiler, file_name):
    compiler.Compile()
    compiler.WriteToFile(file_name)
    del compiler
    d = pykeyvi.Dictionary(file_name)
    yield d
    del d
    os.remove(file_name)