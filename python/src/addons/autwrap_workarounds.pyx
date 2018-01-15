
# import uint32_t type
from libc.stdint cimport uint32_t
from cpython.version cimport PY_MAJOR_VERSION

import json
import msgpack
import keyvi._pycore
import os.path

def get_bin_folder():
    module_location = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(keyvi._pycore.__file__)), ".."))

    if (PY_MAJOR_VERSION >= 3):
        module_location = module_location.encode('utf-8')

    return os.path.join(module_location, b"_bin")