import sys
from keyvi.dictionary import Dictionary
from keyvi.util import FsaTransform

d=Dictionary("normalization.kv")
n=FsaTransform(d)


for line in sys.stdin:
    print(n.Normalize(line))
