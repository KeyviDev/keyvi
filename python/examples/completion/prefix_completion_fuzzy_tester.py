from __future__ import annotations

from keyvi.completion import PrefixCompletion
from keyvi.dictionary import Dictionary

query = ""

d = Dictionary("prefix-completion.kv")
c = PrefixCompletion(d)


def get_lookup_key(query):
    return query


while query != "exit":
    query = str(input("Query:"))
    for m in c.complete_fuzzy(get_lookup_key(query.strip()), 3):
        print("{} {}".format(m.matched_string, m["weight"]))
