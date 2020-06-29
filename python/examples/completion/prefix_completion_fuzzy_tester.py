from keyvi.dictionary import Dictionary
from keyvi.completion import PrefixCompletion

query = ""

d=Dictionary("prefix-completion.kv")
c=PrefixCompletion(d)

def get_lookup_key(query):
    return query

while query!="exit":
    query = str(input("Query:"))
    for m in c.GetFuzzyCompletions(get_lookup_key(query.strip()), 3):
        print("{} {}".format(m.GetMatchedString(), m.GetAttribute("weight")))
