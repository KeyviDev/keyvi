# -*- coding: utf-8 -*-
# Usage: py.test tests

import keyvi

import sys
import os

root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(root, "../"))

from test_tools import tmp_dictionary

def test_forward_backward_completion():
    c = keyvi.CompletionDictionaryCompiler({"memory_limit_mb":"10"})
    c.Add("bayern munich vs. real madrid", 80)
    c.Add("munich vs. real madrid", 30)

    c_bw = keyvi.CompletionDictionaryCompiler({"memory_limit_mb":"10"})
    c_bw.Add("bayern munich vs. real madrid"[::-1], 80)
    c_bw.Add("munich vs. real madrid"[::-1], 30)

    with tmp_dictionary(c, 'fw_bw_completion.kv') as d:
        with tmp_dictionary(c_bw, 'fw_bw_completion_bw.kv') as d2:
            completer = keyvi.ForwardBackwardCompletion(d, d2)
            matches = sorted([(match.GetAttribute('weight'), match.GetMatchedString())
                              for match in completer.GetCompletions("munich")], reverse=True)
            assert len(matches) == 2
            assert matches[0][1] == 'bayern munich vs. real madrid'
            assert matches[1][1] == 'munich vs. real madrid'
