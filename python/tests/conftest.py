from __future__ import annotations

import importlib
import os
import sys

# Redirect all keyvi imports to an alternative module for testing purposes.
# Set KEYVI_MODULE_OVERWRITE to the module name (e.g. "keyvi2") to test
# a different implementation against the existing test suite.
if keyvi_overwrite := os.getenv("KEYVI_MODULE_OVERWRITE"):
    for sub in (
        "",
        ".dictionary",
        ".compiler",
        ".completion",
        ".index",
        ".util",
        ".vector",
    ):
        sys.modules["keyvi" + sub] = importlib.import_module(keyvi_overwrite + sub)
