# -*- coding: utf-8 -*-
# Usage: py.test tests

from test_tools import tmp_dictionary
import sys
import os

from keyvi.compiler import JsonDictionaryCompiler

root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(root, "../"))


def test_near():
    c = JsonDictionaryCompiler({"memory_limit_mb": "10"})
    c.add("zahnarzt:u0we9yykdyum", '["a" : 2]')
    c.add("zahnarzt:u1h2fde2kct3", '["a" : 3]')
    c.add("zahnarzt:u1huf1q5cnxn", '["a" : 4]')
    c.add("zahnarzt:u0y2dvey61sw", '["a" : 5]')
    c.add("zahnarzt:u1hvqmmj801r", '["a" : 6]')
    c.add("zahnarzt:u0vvmknrwgmj", '["a" : 7]')
    c.add("zahnarzt:u0ypv22fb9q3", '["a" : 8]')
    c.add("zahnarzt:u1qcvvw0hxe1", '["a" : 9]')
    c.add("zahnarzt:u1xjx6yfvfz2", '["a" : 10]')
    c.add("zahnarzt:u1q0gkqsenhf", '["a" : 11]')
    with tmp_dictionary(c, 'near_simple.kv') as d:
        assert (len(list(d.match_near("zahnarzt:u1q0gkqsenhf", 12))) == 1)
        assert (len(list(d.match_near("zahnarzt:u1h0gkqsenhf", 12))) == 3)
        assert (len(list(d.match_near("zahnarzt:u1h0gkqsenhf", 13))) == 0)
        assert (len(list(d.match_near("zahnarzt:u0h0gkqsenhf", 10))) == 4)


def test_near_greedy():
    c = JsonDictionaryCompiler({"memory_limit_mb": "10"})
    c.add("zahnarzt:u0we9yykdyum", '["a" : 2]')
    c.add("zahnarzt:u1h2fde2kct3", '["a" : 3]')
    c.add("zahnarzt:u1huf1q5cnxn", '["a" : 4]')
    c.add("zahnarzt:u0y2dvey61sw", '["a" : 5]')
    c.add("zahnarzt:u1hvqmmj801r", '["a" : 6]')
    c.add("zahnarzt:u0vvmknrwgmj", '["a" : 7]')
    c.add("zahnarzt:u0ypv22fb9q3", '["a" : 8]')
    c.add("zahnarzt:u1qcvvw0hxe1", '["a" : 9]')
    c.add("zahnarzt:u1xjx6yfvfz2", '["a" : 10]')
    c.add("zahnarzt:u1q0gkqsenhf", '["a" : 11]')
    with tmp_dictionary(c, 'near_greedy.kv') as d:
        assert (len(list(d.match_near("zahnarzt:u1q0gkqsenhf", 12, True))) == 2)
        assert (len(list(d.match_near("zahnarzt:u1h0gkqsenhf", 12, True))) == 3)
        assert (len(list(d.match_near("zahnarzt:u1h0gkqsenhf", 13, True))) == 0)
        assert (len(list(d.match_near("zahnarzt:u0h0gkqsenhf", 10, True))) == 10)

        greedy = [x.matched_string
                  for x in d.match_near("zahnarzt:u0h0gkqsenhf", 10, True)]
        non_greedy = [x.matched_string
                      for x in d.match_near("zahnarzt:u0h0gkqsenhf", 10, False)]
        assert greedy[:len(non_greedy)] == non_greedy


def test_near_score():
    c = JsonDictionaryCompiler({"memory_limit_mb": "10"})
    c.add("zahnarzt:u0we9yykdyum", '["a" : 2]')
    c.add("zahnarzt:u1h2fde2kct3", '["a" : 3]')
    c.add("zahnarzt:u1huf1q5cnxn", '["a" : 4]')
    c.add("zahnarzt:u0y2dvey61sw", '["a" : 5]')
    c.add("zahnarzt:u1hvqmmj801r", '["a" : 6]')
    c.add("zahnarzt:u0vvmknrwgmj", '["a" : 7]')
    c.add("zahnarzt:u0ypv22fb9q3", '["a" : 8]')
    c.add("zahnarzt:u1qcvvw0hxe1", '["a" : 9]')
    c.add("zahnarzt:u1xjx6yfvfz2", '["a" : 10]')
    c.add("zahnarzt:u1q0gkqsenhf", '["a" : 11]')
    c.add("zahnarzt:u0h0gkqsenhf", '["a" : 11]')

    with tmp_dictionary(c, 'near_score.kv') as d:
        greedy = list(d.match_near("zahnarzt:u0h0gkqsenhf", 10, True))
        assert greedy[0].score == 21
        for m in greedy[1:5]:
            assert m.score == 11
        for m in greedy[5:]:
            assert m.score == 10


def test_near_less_precission():
    c = JsonDictionaryCompiler({"memory_limit_mb": "10"})
    c.add("zahnarzt:u0we9", '["a" : 2]')
    c.add("zahnarzt:u1h2f", '["a" : 3]')
    c.add("zahnarzt:u1huf", '["a" : 4]')
    with tmp_dictionary(c, 'near_less_precission.kv') as d:
        assert (len(list(d.match_near("zahnarzt:u1h0gkqsenhf", 12))) == 2)
        assert (len(list(d.match_near("zahnarzt:u1h0gkqsenhf", 13))) == 0)


def test_near_broken_input():
    c = JsonDictionaryCompiler({"memory_limit_mb": "10"})
    c.add("zahnarzt:u0we9", '["a" : 2]')
    c.add("zahnarzt:u1h2f", '["a" : 3]')
    c.add("zahnarzt:u1huf", '["a" : 4]')
    with tmp_dictionary(c, 'near_broken.kv') as d:
        assert (len(list(d.match_near("zahnarzt:u1h", 12))) == 2)
        assert (len(list(d.match_near("zahnarzt:u", 13))) == 0)
        assert (len(list(d.match_near("zahnarzt:u1", 12))) == 0)
