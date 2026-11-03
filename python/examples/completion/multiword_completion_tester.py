from __future__ import annotations

from keyvi.completion import MultiWordCompletion
from keyvi.dictionary import Dictionary

MULTIWORD_QUERY_SEPARATOR = "\x1b"

query = ""

d = Dictionary("mw-completion.kv")
c = MultiWordCompletion(d)


def get_lookup_key(query):
    tokens = query.split(" ")
    return " ".join(sorted(tokens[:-1]) + tokens[-1:])


while query != "exit":
    query = str(input("Query:"))
    for m in c.complete(get_lookup_key(query.strip())):
        print("{} {}".format(m.matched_string, m["weight"]))
