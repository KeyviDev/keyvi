from keyvi.dictionary import Dictionary
from keyvi.completion import PrefixCompletion

query = ""

d=Dictionary("prefix-completion.keyvi")
c=PrefixCompletion(d)

def get_lookup_key(query):
    return query

while query!="exit":
    query = raw_input("Query:")
    for m in c.GetCompletions(get_lookup_key(query.strip())):
        print("{} ({})".format(m.GetMatchedString(), m.GetAttribute("weight")))
