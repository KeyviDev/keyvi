import sys
import keyvi

d=keyvi.Dictionary("normalization.keyvi")
n=keyvi.FsaTransform(d)


for line in sys.stdin:
    print n.Normalize(line)
