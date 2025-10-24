from keyvi.dictionary import Dictionary
from keyvi.completion import MultiWordCompletion

MULTIWORD_QUERY_SEPARATOR = '\x1b'

query = ""

d=Dictionary("mw-completion.kv")
c=MultiWordCompletion(d)

def get_lookup_key(query):
    l = query.split(" ")
    l_bow = " ".join(sorted(l[:-1]) + l[-1:])

    return l_bow


while query!="exit":
    query = str(input("Query:"))
    for m in c.GetCompletions(get_lookup_key(query.strip())):
        print("{} {}".format(m.GetMatchedString(), m.GetAttribute("weight")))
