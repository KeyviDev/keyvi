# -*- coding: utf-8 -*-
# Usage: py.test tests

import contextlib
import os

import pykeyvi

import sys
import os

root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(root, "../"))

from test_tools import tmp_dictionary

MULTIWORD_QUERY_SEPARATOR = '\x1b'

def test_mw_completion():
    c=pykeyvi.CompletionDictionaryCompiler()
    c.Add("mozilla firefox" + '\x1b' + "mozilla firefox", 80)
    c.Add("mozilla footprint" + '\x1b' + "mozilla footprint", 30)
    c.Add("mozilla fans" + '\x1b' + "mozilla fans", 43)
    c.Add("mozilla firebird" + '\x1b' + "mozilla firebird", 12)
    c.Add("internet microsoft explorer" + '\x1b' + "microsoft internet explorer", 21)
    c.Add("google chrome" + '\x1b' + "google chrome", 54)
    c.Add("netscape navigator" + '\x1b' + "netscape navigator", 10)
    with tmp_dictionary(c, 'mw_completion.kv') as d:
        mw = pykeyvi.MultiWordCompletion(d)
        matches = sorted([(match.GetAttribute('weight'), match.GetMatchedString())
                          for match in mw.GetCompletions("mozilla f")], reverse=True)
        assert len(matches) == 4
        assert matches[0][1] == 'mozilla firefox'
        assert matches[1][1] == 'mozilla fans'
        assert matches[2][1] == 'mozilla footprint'
        assert matches[3][1] == 'mozilla firebird'
