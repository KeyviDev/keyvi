# -*- coding: utf-8 -*-
# Usage: py.test tests

import sys
import os

from keyvi.compiler import CompletionDictionaryCompiler, KeyOnlyDictionaryCompiler
from keyvi.completion import PrefixCompletion

root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(root, "../"))

from test_tools import tmp_dictionary


# from https://github.com/KeyviDev/keyvi/issues/50
def test_fuzzy_completion():
    c = CompletionDictionaryCompiler({"memory_limit_mb": "10"})
    c.add("turkei news", 23698)
    c.add("turkei side", 18838)
    c.add("turkei urlaub", 23424)
    c.add("turkisch anfänger", 20788)
    c.add("turkisch für", 21655)
    c.add("turkisch für anfänger", 20735)
    c.add("turkçe dublaj", 28575)
    c.add("turkçe dublaj izle", 16391)
    c.add("turkçe izle", 19946)
    c.add("tuv", 97)
    c.add("tuv akademie", 9557)
    c.add("tuv hessen", 7744)
    c.add("tuv i", 331)
    c.add("tuv in", 10188)
    c.add("tuv ib", 10189)
    c.add("tuv kosten", 11387)
    c.add("tuv nord", 46052)
    c.add("tuv sood", 46057)
    c.add("tus rhein", 462)
    c.add("tus rheinland", 39131)
    c.add("tus öffnungszeiten", 15999)

    with tmp_dictionary(c, "fuzzy_completion.kv") as d:
        completer = PrefixCompletion(d)
        matches = [m.matched_string for m in completer.complete_fuzzy("tuv", 0)]
        assert len(matches) == 9

        matches = [m.matched_string for m in completer.complete_fuzzy("tue", 1)]
        assert len(matches) == 21

        matches = [m.matched_string for m in completer.complete_fuzzy("tuv h", 1)]
        assert len(matches) == 8

        matches = [m.matched_string for m in completer.complete_fuzzy("tuv h", 2)]
        assert len(matches) == 12

        matches = [
            m.matched_string for m in completer.complete_fuzzy("tuk töffnungszeiten", 2)
        ]
        assert len(matches) == 1

        matches = [
            m.matched_string for m in completer.complete_fuzzy("tuk töffnung", 2)
        ]
        assert len(matches) == 1

        matches = [m.matched_string for m in completer.complete_fuzzy("tuk txyzöff", 5)]
        assert len(matches) == 1

        matches = [
            m.matched_string for m in completer.complete_fuzzy("tuk txyzöffnung", 5)
        ]
        assert len(matches) == 1

        matches = [
            m.matched_string for m in completer.complete_fuzzy("tuk txyzvöffnung", 6)
        ]
        assert len(matches) == 1

        matches = [m.matched_string for m in completer.complete_fuzzy("tuk ffnung", 2)]
        assert len(matches) == 1


def test_fuzzy_completion_utf8():
    c = KeyOnlyDictionaryCompiler()
    c.add("mß")

    with tmp_dictionary(c, "fuzzy_completion_utf8.kv") as d:
        completer = PrefixCompletion(d)

        matches = [m.matched_string for m in completer.complete_fuzzy("mß", 1)]
        assert len(matches) == 1

        matches = [m.matched_string for m in completer.complete_fuzzy("mß", 1, 0)]
        assert len(matches) == 1

        matches = [m.matched_string for m in completer.complete_fuzzy("mß", 1, 4)]
        assert len(matches) == 1
