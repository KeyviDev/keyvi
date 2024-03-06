# -*- coding: utf-8 -*-
# Usage: py.test tests

import heapq
import sys
import os

from keyvi.compiler import CompletionDictionaryCompiler

root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(root, "../"))
from test_tools import tmp_dictionary


def test_prefix_simple():
    c = CompletionDictionaryCompiler({"memory_limit_mb": "10"})
    c.Add("eric", 33)
    c.Add("jeff", 33)
    c.Add("eric bla", 233)
    c.Add("eric blu", 113)
    c.Add("eric ble", 413)
    c.Add("eric blx", 223)
    c.Add("eric bllllx", 193)
    c.Add("eric bxxxx", 23)
    c.Add("eric boox", 143)
    with tmp_dictionary(c, "completion.kv") as d:
        assert [m.matched_string for m in d.complete_prefix("eric")] == [
            "eric",
            "eric ble",
            "eric bla",
            "eric blx",
            "eric bllllx",
            "eric blu",
            "eric boox",
            "eric bxxxx",
        ]

        # note: we are getting one more("eric"), because of dfs traversal
        assert [m.matched_string for m in d.complete_prefix("eric", 2)] == [
            "eric",
            "eric ble",
            "eric bla",
        ]

        def filter_x(completer):
            for m in completer:
                if m.matched_string.endswith("x"):
                    completer.set_min_weight(40)
                    yield m

        assert [m.matched_string for m in filter_x(d.complete_prefix("eric"))] == [
            "eric blx",
            "eric bllllx",
            "eric boox",
        ]

        class TopNFilter:
            def __init__(self, n) -> None:
                self.n = n
                self.heap = []
                self.visits = 0

            def filter(self, completer):
                for m in completer:
                    assert m.weight == m.value
                    self.visits += 1
                    if len(self.heap) < self.n:
                        heapq.heappush(self.heap, m.weight)
                        yield m
                    elif m.weight > self.heap[0]:
                        heapq.heappop(self.heap)
                        heapq.heappush(self.heap, m.weight)
                        completer.set_min_weight(self.heap[0])
                        yield m

        top5 = TopNFilter(5)
        # note: we are getting more erics, because of dfs traversal
        assert [m.matched_string for m in top5.filter(d.complete_prefix("eric"))] == [
            "eric",
            "eric ble",
            "eric bla",
            "eric blx",
            "eric bllllx",
            "eric blu",
            "eric boox",
        ]

        # by traversing using min weight, we should _not_ visit all entries
        assert top5.visits < len(d)

        top3 = TopNFilter(3)
        # note: getting more erics, because of dfs traversal
        assert [m.matched_string for m in top3.filter(d.complete_prefix("eric"))] == [
            "eric",
            "eric ble",
            "eric bla",
            "eric blx",
        ]

        # top-3 should have less visits than top-5
        assert top3.visits < top5.visits


def test_mismatches():
    c = CompletionDictionaryCompiler({"memory_limit_mb": "10"})
    c.Add("a", 33)
    c.Add("ab", 33)
    c.Add("abcd", 233)
    with tmp_dictionary(c, "completion.kv") as d:
        assert [m.matched_string for m in d.complete_prefix("v")] == []
        assert [m.matched_string for m in d.complete_prefix("vwxyz")] == []
        assert [m.matched_string for m in d.complete_prefix("av")] == []
        assert [m.matched_string for m in d.complete_prefix("abcde")] == []
        assert [m.matched_string for m in d.complete_prefix(" ")] == []
