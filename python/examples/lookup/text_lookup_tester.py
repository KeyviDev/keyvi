from __future__ import annotations

from keyvi.dictionary import Dictionary

query = ""

d = Dictionary("cities.kv")


def get_lookup_key(query):
    return query


while query != "exit":
    query = input("Query:")
    for m in d.search_tokenized(get_lookup_key(query.strip())):
        print(f"{m.matched_string}")
