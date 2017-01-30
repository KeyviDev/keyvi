# -*- coding: utf-8 -*-
# Usage: py.test tests

import os
import tempfile
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

def test_manifest_after_compile():
    c = pykeyvi.KeyOnlyDictionaryCompiler()
    c.Add("Leela")
    c.Add("Kif")
    c.Compile()
    c.SetManifest({"author": "Zapp Brannigan"})
    file_name = os.path.join(tempfile.gettempdir(),'brannigan_manifest2.kv')
    try:
        c.WriteToFile(file_name)
        d = pykeyvi.Dictionary(file_name)
        m = d.GetManifest()
        assert m['author'] == "Zapp Brannigan"
        del d
    finally:
        os.remove(file_name)

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

def test_manifest_for_merger():
    try:
        c = pykeyvi.JsonDictionaryCompiler()
        c.Add("abc", '{"a" : 2}')
        c.Compile()
        c.SetManifest({"author": "Zapp Brannigan"})
        c.WriteToFile('manifest_json_merge1.kv')
        del c

        c2 = pykeyvi.JsonDictionaryCompiler()
        c2.Add("abd", '{"a" : 3}')
        c2.Compile()
        c2.SetManifest({"author": "Leela"})
        c2.WriteToFile('manifest_json_merge2.kv')
        del c2

        merger = pykeyvi.JsonDictionaryMerger()
        merger.SetManifest({"author": "Fry"})
        merger.Merge('manifest_json_merged.kv')

        d = pykeyvi.Dictionary('manifest_json_merged.kv')
        m = d.GetManifest()
        assert m['author'] == "Fry"
        del d

    finally:
        os.remove('manifest_json_merge1.kv')
        os.remove('manifest_json_merge2.kv')
        os.remove('manifest_json_merged.kv')
