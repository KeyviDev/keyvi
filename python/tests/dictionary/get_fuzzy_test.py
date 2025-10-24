# -*- coding: utf-8 -*-
# Usage: py.test tests

from keyvi.compiler import CompletionDictionaryCompiler, IntDictionaryCompiler

from test_tools import tmp_dictionary


def test_match_fuzzy():
    c = CompletionDictionaryCompiler({"memory_limit_mb": "10"})
    c.add("türkei news", 23698)
    c.add("türkei side", 18838)
    c.add("türkei urlaub", 23424)
    c.add("türkisch anfänger", 20788)
    c.add("türkisch für", 21655)
    c.add("türkisch für anfänger", 20735)
    c.add("türkçe dublaj", 28575)
    c.add("türkçe dublaj izle", 16391)
    c.add("türkçe izle", 19946)
    c.add("tüv akademie", 9557)
    c.add("tüv hessen", 7744)
    c.add("tüv i", 331)
    c.add("tüv in", 10188)
    c.add("tüv ib", 10189)
    c.add("tüv kosten", 11387)
    c.add("tüv nord", 46052)
    c.add("tüv sood", 46057)
    c.add("tüs rhein", 462)
    c.add("tüs rheinland", 39131)
    c.add("tüs öffnungszeiten", 15999)

    key_values = [
        ("tüv sood", 46057),
        ("tüv nord", 46052),
    ]

    with tmp_dictionary(c, "match_fuzzy.kv") as d:
        for (base_key, base_value), m in zip(key_values, d.match_fuzzy("tüv koid", 2)):
            assert base_key == m.matched_string
            assert base_value == m.value

        assert len(list(d.match_fuzzy("tüv koid", 2))) == 2


def test_match_fuzzy_minimum_prefix():
    c = IntDictionaryCompiler({"memory_limit_mb": "10"})
    c.add("a", 0)
    c.add("apple", 1)
    with tmp_dictionary(c, "match_fuzzy_mp.kv") as d:
        matches = list(d.match_fuzzy("app", 0, 1))
        assert len(matches) == 0
        matches = list(d.match_fuzzy("ap", 1, 1))
        assert len(matches) == 1
        assert matches[0].value == 0
        assert matches[0].matched_string == "a"
        assert matches[0].score == 1
