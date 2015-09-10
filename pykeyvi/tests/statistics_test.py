# -*- coding: utf-8 -*-

import os

import pykeyvi

def size_test():
    c = pykeyvi.KeyOnlyDictionaryCompiler()
    c.Add("Leela")
    c.Add("Kif")
    c.Compile()
    c.WriteToFile('brannigan_1.kv')
    d = pykeyvi.Dictionary('brannigan_1.kv')
    os.remove('brannigan_1.kv')
    assert len(d) == 2

def manifest_test():
    c = pykeyvi.KeyOnlyDictionaryCompiler()
    c.Add("Leela")
    c.Add("Kif")
    c.Compile()
    c.SetManifest({"author": "Zapp Brannigan"})
    c.WriteToFile('brannigan_2.kv')
    d = pykeyvi.Dictionary('brannigan_2.kv')
    os.remove('brannigan_2.kv')
    m = d.GetManifest()
    assert m['author'] == "Zapp Brannigan"

def statistics_test():
    c = pykeyvi.KeyOnlyDictionaryCompiler()
    c.Add("Leela")
    c.Add("Kif")
    c.Compile()
    c.SetManifest({"author": "Zapp Brannigan"})
    c.WriteToFile('brannigan_3.kv')
    d = pykeyvi.Dictionary('brannigan_3.kv')
    os.remove('brannigan_3.kv')
    stats = d.GetStatistics()
    gen = stats.get('General', {})
    man = gen.get('manifest', {})
    size = int(gen.get('number_of_keys', 0))

    assert size == 2
    assert man.get('author') == "Zapp Brannigan"
