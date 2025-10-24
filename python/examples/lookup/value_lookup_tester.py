from keyvi.dictionary import Dictionary

query = ""

d=Dictionary("your-own.kv")

def get_lookup_key(query):
    return query

while query!="exit":
    query = raw_input("Query:")
    for m in d.Get(get_lookup_key(query.strip())):
        print("{} {}".format(m.GetMatchedString(), m.GetValueAsString()))
