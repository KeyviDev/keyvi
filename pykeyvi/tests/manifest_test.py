# -*- coding: utf-8 -*-

import pykeyvi
import json

def manifest_test():
    compiler = pykeyvi.KeyOnlyDictionaryCompiler()
    c.Add("Leela")
    c.Add("Kif")
    c.Compile()
    c.SetManifest('{"author": "Zapp Brannigan"}')
    c.WriteToFile('brannigan_1.kv')
    d = pykeyvi.Dictionary('brannigan_1.kv')
    m=json.loads(d.GetManifestAsString())
    assert m['author'] == "Zapp Brannigan"