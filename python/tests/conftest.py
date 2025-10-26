from __future__ import annotations

import importlib
import os
import sys


# patch keyvi imports to an alternative module for testing purposes
if keyvi2_module_name := os.getenv("KEYVI_MODULE_OVERWRITE"):
    for sub in ("", ".dictionary", ".compiler", ".completion"):
        sub_module_name = "keyvi" + sub
        keyvi2_sub_module_name = keyvi2_module_name + sub
        sys.modules[sub_module_name] = importlib.import_module(keyvi2_sub_module_name)
