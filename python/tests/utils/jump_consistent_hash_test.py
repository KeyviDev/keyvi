from __future__ import annotations

from keyvi.util import JumpConsistentHashString


def test_jump_consistent_hash():
    assert JumpConsistentHashString("some string", 117) == 60
