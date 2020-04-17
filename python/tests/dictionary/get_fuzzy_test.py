# -*- coding: utf-8 -*-
# Usage: py.test tests


from keyvi.compiler import CompletionDictionaryCompiler

from test_tools import tmp_dictionary


def test_get_fuzzy():
    c = CompletionDictionaryCompiler({"memory_limit_mb": "10"})
    c.Add("türkei news", 23698)
    c.Add("türkei side", 18838)
    c.Add("türkei urlaub", 23424)
    c.Add("türkisch anfänger", 20788)
    c.Add("türkisch für", 21655)
    c.Add("türkisch für anfänger", 20735)
    c.Add("türkçe dublaj", 28575)
    c.Add("türkçe dublaj izle", 16391)
    c.Add("türkçe izle", 19946)
    c.Add("tüv akademie", 9557)
    c.Add("tüv hessen", 7744)
    c.Add("tüv i", 331)
    c.Add("tüv in", 10188)
    c.Add("tüv ib", 10189)
    c.Add("tüv kosten", 11387)
    c.Add("tüv nord", 46052)
    c.Add("tüv sood", 46057)
    c.Add("tüs rhein", 462)
    c.Add("tüs rheinland", 39131)
    c.Add("tüs öffnungszeiten", 15999)

    key_values = [
        (u'tüv sood', 46057),
        (u'tüv nord', 46052),
    ]

    with tmp_dictionary(c, 'get_fuzzy.kv') as d:
        for (base_key, base_value), m in zip(key_values, d.GetFuzzy('tüv koid', 2)):
            assert base_key == m.GetMatchedString()
            assert base_value == m.GetValue()

        assert len(list(d.GetFuzzy('tüv koid', 2))) == 2
