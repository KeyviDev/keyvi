# -*- coding: utf-8 -*-
# Usage: py.test tests

import pykeyvi
from test_tools import tmp_dictionary

def test_size():
    c = pykeyvi.KeyOnlyDictionaryCompiler()
    c.Add("Leela")
    c.Add("Kif")
    with tmp_dictionary(c, 'brannigan_size.kv') as d:
        assert len(d) == 2

def test_manifest():
    c = pykeyvi.KeyOnlyDictionaryCompiler()
    c.Add("Leela")
    c.Add("Kif")
    c.SetManifest({"author": "Zapp Brannigan"})
    with tmp_dictionary(c, 'brannigan_manifest.kv') as d:
        m = d.GetManifest()
        assert m['author'] == "Zapp Brannigan"

def test_statistics():
    c = pykeyvi.KeyOnlyDictionaryCompiler()
    c.Add("Leela")
    c.Add("Kif")
    c.SetManifest({"author": "Zapp Brannigan"})
    with tmp_dictionary(c, 'brannigan_statistics.kv') as d:
        stats = d.GetStatistics()
        gen = stats.get('General', {})
        man = gen.get('manifest', {})
        size = int(gen.get('number_of_keys', 0))
        assert size == 2
        assert man.get('author') == "Zapp Brannigan"
