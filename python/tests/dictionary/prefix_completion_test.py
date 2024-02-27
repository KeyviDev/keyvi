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

        heap = []

        def filter_top_5(completer):
            for m in completer:
                if len(heap) < 6:
                    heapq.heappush(heap, m.score)
                    yield m
                elif m.score > heap[0]:
                    heapq.heappop(heap)
                    heapq.heappush(heap, m.score)
                    completer.set_min_weight(m.score)
                    yield m

        # note: we are getting one more("eric"), because of dfs traversal
        assert [m.matched_string for m in filter_top_5(d.complete_prefix("eric"))] == [
            "eric",
            "eric ble",
            "eric bla",
            "eric blx",
            "eric bllllx",
            "eric blu",
        ]
