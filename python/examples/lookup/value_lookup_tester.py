from __future__ import annotations

from keyvi.dictionary import Dictionary

query = ""

d = Dictionary("your-own.kv")


def get_lookup_key(query):
    return query


while query != "exit":
    query = input("Query:")
    for m in d.get(get_lookup_key(query.strip())):
        print(f"{m.matched_string} {m.value_as_string()}")
