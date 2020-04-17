# -*- coding: utf-8 -*-
# Usage: py.test tests
import sys
import os

from keyvi.compiler import JsonDictionaryCompiler

root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(root, "../"))

from test_tools import tmp_dictionary

def test_simple():
    c = JsonDictionaryCompiler({"memory_limit_mb":"10"})
    c.Add("abc", '{"a" : 2}')
    c.Add("abd", '{"a" : 3}')
    # use python syntax ala __setitem__
    c["abd"] = '{"a" : 3}'
    with tmp_dictionary(c, 'simple_json.kv') as d:
        assert len(d) == 2
        assert d["abc"].GetValueAsString() == '{"a":2}'
        assert d["abd"].GetValueAsString() == '{"a":3}'

def test_simple_zlib():
    c = JsonDictionaryCompiler({"memory_limit_mb":"10", 'compression': 'z', 'compression_threshold': '0'})
    c.Add("abc", '{"a" : 2}')
    c.Add("abd", '{"a" : 3}')
    with tmp_dictionary(c, 'simple_json_z.kv') as d:
        assert len(d) == 2
        assert d["abc"].GetValueAsString() == '{"a":2}'
        assert d["abd"].GetValueAsString() == '{"a":3}'
        m = d.GetStatistics()['Value Store']
        assert m['__compression'] == "zlib"

def test_simple_snappy():
    c = JsonDictionaryCompiler({"memory_limit_mb":"10", 'compression': 'snappy', 'compression_threshold': '0'})
    c.Add("abc", '{"a" : 2}')
    c.Add("abd", '{"a" : 3}')
    with tmp_dictionary(c, 'simple_json_snappy.kv') as d:
        assert len(d) == 2
        assert d["abc"].GetValueAsString() == '{"a":2}'
        assert d["abd"].GetValueAsString() == '{"a":3}'
        m = d.GetStatistics()['Value Store']
        assert m['__compression'] == "snappy"

def test_unicode_compile():
    c = JsonDictionaryCompiler({"memory_limit_mb":"10"})
    c.Add("üöä", '{"y" : 2}')
    c.Add("üüüüüüabd", '{"a" : 3}')
    c.Add(u"ääääädäd", '{"b" : 33}')

    with tmp_dictionary(c, 'simple_json.kv') as d:
        assert len(d) == 3
        assert d["üöä"].GetValueAsString() == '{"y":2}'
        assert d[u"üöä"].GetValueAsString() == '{"y":2}'
        assert d["üüüüüüabd"].GetValueAsString() == '{"a":3}'
        assert d["ääääädäd"].GetValueAsString() == '{"b":33}'

def test_float_compaction():
    cs = JsonDictionaryCompiler({"memory_limit_mb":"10", 'floating_point_precision': 'single'})
    cd = JsonDictionaryCompiler({"memory_limit_mb":"10"})

    # add a couple of floats to both
    cs.Add('aa', '[1.7008715758978892, 1.8094465532317732, 1.6098250864350536, 1.6369107966501981, 1.7736887965234107, 1.606682751740542, 1.6186427703265525, 1.7939763843449683, 1.5973550162469434, 1.6799721708726192, 1.8199786239525833, 1.7956178070065245, 1.7269879953863045]')
    cd.Add('aa', '[1.7008715758978892, 1.8094465532317732, 1.6098250864350536, 1.6369107966501981, 1.7736887965234107, 1.606682751740542, 1.6186427703265525, 1.7939763843449683, 1.5973550162469434, 1.6799721708726192, 1.8199786239525833, 1.7956178070065245, 1.7269879953863045]')

    with tmp_dictionary(cs, 'json_single_precision_float.kv') as ds:
        with tmp_dictionary(cd, 'json_double_precision_float.kv') as dd:
            # first some basic checks
            assert len(ds) == 1
            assert len(dd) == 1
            # simple test the length of the value store which shall be smaller for single floats
            stats_s = ds.GetStatistics()
            stats_d = dd.GetStatistics()
            assert int(stats_s['Value Store']['size']) < int(stats_d['Value Store']['size'])
