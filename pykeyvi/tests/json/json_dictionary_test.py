# -*- coding: utf-8 -*-
# Usage: py.test tests

import contextlib
import os

import pykeyvi

import sys
import os

root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(root, "../"))

from test_tools import tmp_dictionary, decode_to_unicode

def test_simple():
    c = pykeyvi.JsonDictionaryCompiler()
    c.Add("abc", '{"a" : 2}')
    c.Add("abd", '{"a" : 3}')
    # use python syntax ala __setitem__
    c["abd"] = '{"a" : 3}'
    with tmp_dictionary(c, 'simple_json.kv') as d:
        assert len(d) == 2
        assert decode_to_unicode(d["abc"].GetValueAsString()) == decode_to_unicode('{"a":2}')
        assert decode_to_unicode(d["abd"].GetValueAsString()) == decode_to_unicode('{"a":3}')

def test_simple_zlib():
    c = pykeyvi.JsonDictionaryCompiler(50000000, {'compression': 'z', 'compression_threshold': '0'})
    c.Add("abc", '{"a" : 2}')
    c.Add("abd", '{"a" : 3}')
    with tmp_dictionary(c, 'simple_json_z.kv') as d:
        assert len(d) == 2
        assert decode_to_unicode(d["abc"].GetValueAsString()) == decode_to_unicode('{"a":2}')
        assert decode_to_unicode(d["abd"].GetValueAsString()) == decode_to_unicode('{"a":3}')
        m = d.GetStatistics()['Value Store']
        assert m['__compression'] == decode_to_unicode("zlib")
        assert m['__compression_threshold'] == decode_to_unicode("0")

def test_simple_snappy():
    c = pykeyvi.JsonDictionaryCompiler(50000000, {'compression': 'snappy', 'compression_threshold': '0'})
    c.Add("abc", '{"a" : 2}')
    c.Add("abd", '{"a" : 3}')
    with tmp_dictionary(c, 'simple_json_snappy.kv') as d:
        assert len(d) == 2
        assert decode_to_unicode(d["abc"].GetValueAsString()) == decode_to_unicode('{"a":2}')
        assert decode_to_unicode(d["abd"].GetValueAsString()) == decode_to_unicode('{"a":3}')
        m = d.GetStatistics()['Value Store']
        assert m['__compression'] == "snappy"
        assert m['__compression_threshold'] == "0"

def test_unicode_compile():
    c = pykeyvi.JsonDictionaryCompiler()
    c.Add("üöä", '{"y" : 2}')
    c.Add(decode_to_unicode("üüüüüüabd"), '{"a" : 3}')
    c.Add(u"ääääädäd", '{"b" : 33}')

    with tmp_dictionary(c, 'simple_json.kv') as d:
        assert len(d) == 3
        assert decode_to_unicode(d["üöä"].GetValueAsString()) == decode_to_unicode('{"y":2}')
        assert decode_to_unicode(d[u"üöä"].GetValueAsString()) == decode_to_unicode('{"y":2}')
        assert decode_to_unicode(d["üüüüüüabd"].GetValueAsString()) == decode_to_unicode('{"a":3}')
        assert decode_to_unicode(d["ääääädäd"].GetValueAsString()) == decode_to_unicode('{"b":33}')

def test_float_compaction():
    cs = pykeyvi.JsonDictionaryCompiler(50000000, {'floating_point_precision': 'single'})
    cd = pykeyvi.JsonDictionaryCompiler(50000000)

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
