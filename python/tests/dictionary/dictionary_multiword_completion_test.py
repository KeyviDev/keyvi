# -*- coding: utf-8 -*-
# Usage: py.test tests

import sys
import os
from functools import reduce

from keyvi.compiler import CompletionDictionaryCompiler

root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(root, "../"))
from test_tools import tmp_dictionary


multiword_data = {
    "80s action film released 2013": {"w": 43, "id": "a1"},
    "80s action heros": {"w": 72, "id": "a2"},
    "80s back": {"w": 1, "id": "a3"},
    "80s baladen": {"w": 37, "id": "a4"},
    "80s cartoon with cars": {"w": 2, "id": "a5"},
    "80s game of thrones theme": {"w": 3, "id": "a6"},
    "80s girl group": {"w": 4, "id": "a7"},
    "80s hard rock berlin": {"w": 66, "id": "a8"},
    "80s indie songs": {"w": 108, "id": "a9"},
    "80s jack the ripper documentary": {"w": 39, "id": "aa"},
    "80s monsters tribute art": {"w": 13, "id": "ab"},
    "80s movie with zombies": {"w": 67, "id": "ac"},
    "80s overall": {"w": 5, "id": "ad"},
    "80s punk oi last fm": {"w": 33, "id": "ae"},
    "80s retro shop los angeles": {"w": 13, "id": "af"},
    "80s singer roger": {"w": 13, "id": "b1"},
    "80s techno fashion": {"w": 6, "id": "b2"},
    "80s theme party cupcake": {"w": 42, "id": "b3"},
    "80s video megamix": {"w": 96, "id": "b4"},
}

PERMUTATION_LOOKUP_TABLE = {
    2: [
        [0],
        [1],
        [0, 1],
        [1, 0],
    ],
    3: [
        [0],
        [1],
        [2],
        [0, 1],
        [0, 2],
        [1, 0],
        [1, 2],
        [2, 0],
        [2, 1],
        [0, 1, 2],
        [0, 2, 1],
        [1, 2, 0],
    ],
    4: [
        [0],
        [1],
        [2],
        [3],
        [0, 1],
        [0, 2],
        [0, 3],
        [1, 0],
        [1, 2],
        [1, 3],
        [2, 0],
        [2, 1],
        [2, 3],
        [3, 0],
        [3, 1],
        [3, 2],
        [0, 1, 2],
        [0, 1, 3],
        [0, 2, 1],
        [0, 2, 3],
        [0, 3, 1],
        [0, 3, 2],
        [1, 2, 0],
        [1, 2, 3],
        [1, 3, 0],
        [1, 3, 2],
        [2, 3, 0],
        [2, 3, 1],
        [0, 1, 2, 3],
    ],
}

MULTIWORD_QUERY_SEPARATOR = "\x1b"


class MultiWordPermutation:
    def __init__(self):
        pass

    def __call__(self, key_value):
        key, value = key_value
        tokens = key.split(" ")
        tokens_bow = sorted(tokens)

        length = len(tokens_bow)
        if length not in PERMUTATION_LOOKUP_TABLE:
            yield " ".join(tokens) + MULTIWORD_QUERY_SEPARATOR + value
            return

        for permutation in PERMUTATION_LOOKUP_TABLE[len(tokens_bow)]:
            if len(permutation) < 3:
                first_token = tokens_bow[permutation[0]]
                if first_token != tokens[permutation[0]] and len(first_token) == 1:
                    continue
            yield (
                " ".join([tokens_bow[i] for i in permutation])
                + MULTIWORD_QUERY_SEPARATOR
                + value
            )


def test_multiword_simple():
    pipeline = []
    pipeline.append(MultiWordPermutation())
    c = CompletionDictionaryCompiler()

    for key, value in multiword_data.items():
        weight = value["w"]

        for e in reduce(lambda x, y: y(x), pipeline, (key, key)):
            c.Add(e, weight)

    with tmp_dictionary(c, "completion.kv") as d:
        assert [m.matched_string for m in d.complete_multiword("zombies 8")] == [
            "80s movie with zombies"
        ]
        assert [m.matched_string for m in d.complete_multiword("80s mo")] == [
            "80s movie with zombies",
            "80s monsters tribute art",
        ]
        assert [m.matched_string for m in d.complete_multiword("with 80")] == [
            "80s movie with zombies",
            "80s cartoon with cars",
        ]
        assert [m.matched_string for m in d.complete_multiword("techno fa")] == [
            "80s techno fashion",
        ]
        assert [m.matched_string for m in d.complete_multiword("90s")] == []

        assert [
            m.matched_string
            for m in sorted(
                [m for m in d.complete_multiword("80")],
                key=lambda m: m.weight,
                reverse=True,
            )
        ] == [
            k
            for k, v in sorted(
                multiword_data.items(), key=lambda item: item[1]["w"], reverse=True
            )
            if len(k.split(" ")) < 5
        ]
