# -*- coding: utf-8 -*-
# Usage: py.test tests

import pykeyvi

def test_compiler_no_compile_edge_case():
    c = pykeyvi.KeyOnlyDictionaryCompiler()
    c.Add("abc")
    c.Add("abd")
    del c
