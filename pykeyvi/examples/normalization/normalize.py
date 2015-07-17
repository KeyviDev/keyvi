import sys
import pykeyvi

d=pykeyvi.Dictionary("normalization.keyvi")
n=pykeyvi.FsaTransform(d)


for line in sys.stdin:
    print n.Normalize(line)
