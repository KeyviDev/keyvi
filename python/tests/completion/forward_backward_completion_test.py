# -*- coding: utf-8 -*-
# Usage: py.test tests

import sys
import os

from keyvi.compiler import CompletionDictionaryCompiler
from keyvi.completion import ForwardBackwardCompletion

root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(root, "../"))

from test_tools import tmp_dictionary

def test_forward_backward_completion():
    c = CompletionDictionaryCompiler({"memory_limit_mb":"10"})
    c.add("bayern munich vs. real madrid", 80)
    c.add("munich vs. real madrid", 30)

    c_bw = CompletionDictionaryCompiler({"memory_limit_mb":"10"})
    c_bw.add("bayern munich vs. real madrid"[::-1], 80)
    c_bw.add("munich vs. real madrid"[::-1], 30)

    with tmp_dictionary(c, 'fw_bw_completion.kv') as d:
        with tmp_dictionary(c_bw, 'fw_bw_completion_bw.kv') as d2:
            completer = ForwardBackwardCompletion(d, d2)
            matches = sorted([(match['weight'], match.matched_string)
                              for match in completer.GetCompletions("munich")], reverse=True)
            assert len(matches) == 2
            assert matches[0][1] == 'bayern munich vs. real madrid'
            assert matches[1][1] == 'munich vs. real madrid'
