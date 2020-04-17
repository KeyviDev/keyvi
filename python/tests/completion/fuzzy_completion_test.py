# -*- coding: utf-8 -*-
# Usage: py.test tests

import sys
import os

from keyvi.compiler import CompletionDictionaryCompiler
from keyvi.completion import PrefixCompletion

root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(root, "../"))

from test_tools import tmp_dictionary

# from https://github.com/KeyviDev/keyvi/issues/50
def test_fuzzy_completion():
    c = CompletionDictionaryCompiler({"memory_limit_mb": "10"})
    c.Add("turkei news", 23698)
    c.Add("turkei side", 18838)
    c.Add("turkei urlaub", 23424)
    c.Add("turkisch anfänger", 20788)
    c.Add("turkisch für", 21655)
    c.Add("turkisch für anfänger", 20735)
    c.Add("turkçe dublaj", 28575)
    c.Add("turkçe dublaj izle", 16391)
    c.Add("turkçe izle", 19946)
    c.Add("tuv", 97)
    c.Add("tuv akademie", 9557)
    c.Add("tuv hessen", 7744)
    c.Add("tuv i", 331)
    c.Add("tuv in", 10188)
    c.Add("tuv ib", 10189)
    c.Add("tuv kosten", 11387)
    c.Add("tuv nord", 46052)
    c.Add("tuv sood", 46057)
    c.Add("tus rhein", 462)
    c.Add("tus rheinland", 39131)
    c.Add("tus öffnungszeiten", 15999)

    with tmp_dictionary(c, 'fuzzy_completion.kv') as d:
        completer = PrefixCompletion(d)
        matches = [m.GetMatchedString() for m in completer.GetFuzzyCompletions('tuv', 0)]
        assert len(matches) == 9

        matches = [m.GetMatchedString() for m in completer.GetFuzzyCompletions('tue', 1)]
        assert len(matches) == 1

        matches = [m.GetMatchedString() for m in completer.GetFuzzyCompletions('tuv h', 1)]
        assert len(matches) == 2

        matches = [m.GetMatchedString() for m in completer.GetFuzzyCompletions('tuv h', 2)]
        assert len(matches) == 7

        matches = [m.GetMatchedString() for m in completer.GetFuzzyCompletions('tuk töffnungszeiten', 2)]
        assert len(matches) == 1

        matches = [m.GetMatchedString() for m in completer.GetFuzzyCompletions('tuk töffnung', 2)]
        assert len(matches) == 1

        matches = [m.GetMatchedString() for m in completer.GetFuzzyCompletions('tuk txyzöff', 5)]
        assert len(matches) == 1

        matches = [m.GetMatchedString() for m in completer.GetFuzzyCompletions('tuk txyzöffnung', 5)]
        assert len(matches) == 1

        matches = [m.GetMatchedString() for m in completer.GetFuzzyCompletions('tuk txyzvöffnung', 6)]
        assert len(matches) == 1

        matches = [m.GetMatchedString() for m in completer.GetFuzzyCompletions('tuk ffnung', 2)]
        assert len(matches) == 1
