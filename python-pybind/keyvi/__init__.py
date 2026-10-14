from __future__ import annotations

import sys

from keyvi._core import (
    CompressionAlgorithm as CompressionAlgorithm,
)
from keyvi._core import (
    Match as Match,
)
from keyvi._core import __version__ as __version__
from keyvi._core import (
    compiler,
    completion,
    dictionary,
    index,
    util,
    vector,
)
from keyvi._core import (
    loading_strategy_types as loading_strategy_types,
)

sys.modules[__name__ + ".dictionary"] = dictionary
sys.modules[__name__ + ".compiler"] = compiler
sys.modules[__name__ + ".completion"] = completion
sys.modules[__name__ + ".index"] = index
sys.modules[__name__ + ".util"] = util
sys.modules[__name__ + ".vector"] = vector
