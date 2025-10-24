# -*- coding: utf-8 -*-

"""
Query permutator to match multiword queries.

Given a query with n tokens we assume the first n-1 to be complete while the last might be partial. From that we
want to match using the following algorithm.

Bag of words reordering is applied to the first n-1 tokens, n-th token (partial) is kept last.

Example: "messenger facebook deu" -> "facebook messenger deu"

In order to match the index entry: "facebook messenger download deutsch" we need an entry:

"facebook messenger deutsch" which corresponds to tokens 0 1 3

We do not require "messenger facebook deu" (1 0 3) due to the BOW reordering at lookup

"""

import sys
from functools import reduce
from keyvi.compiler import CompletionDictionaryCompiler

# permutation lookup table, number_of_tokens - > permutations_to_build
# the table is build with scripts/build_bow_completion_permutations
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

MULTIWORD_QUERY_SEPARATOR = '\x1b'

class MultiWordPermutation:
    def __init__(self):
        pass

    def __call__(self, query):
        query_tokens = query.split(" ")
        query_tokens_bow = sorted(query_tokens)
        length = len(query_tokens_bow)
        if length not in PERMUTATION_LOOKUP_TABLE:
            yield query
            return

        for permutation in PERMUTATION_LOOKUP_TABLE[len(query_tokens_bow)]:
            if len(permutation) < 3:
                first_token = query_tokens_bow[permutation[0]]
                if first_token != query_tokens[permutation[0]] and len(first_token) == 1:
                    continue
            yield " ".join([query_tokens_bow[i] for i in permutation]) + MULTIWORD_QUERY_SEPARATOR + query


if __name__ == '__main__':
    pipeline = []
    pipeline.append(MultiWordPermutation())
    c = CompletionDictionaryCompiler()


    for line in sys.stdin:
        key, weight = line.split("\t")

        for q in reduce(lambda x, y: y(x), pipeline, key):
            c.Add(q, int(weight))
    c.Compile()
    c.WriteToFile("mw-completion.kv")
