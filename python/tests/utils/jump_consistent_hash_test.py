# -*- coding: utf-8 -*-
# Usage: py.test tests

from keyvi.util import JumpConsistentHashString
import sys


def test_jump_consistent_hash():
    assert JumpConsistentHashString('some string', 117) == 60

    # test unicode on Python 2 only
    if sys.version_info[0] == 2:
        assert JumpConsistentHashString(u'some string', 117) == 60
