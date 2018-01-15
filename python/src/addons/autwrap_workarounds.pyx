
# import uint32_t type
from libc.stdint cimport uint32_t

import json
import msgpack
import keyvi._pycore
import os.path

def get_bin_folder():
    return os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(keyvi._pycore.__file__)), "..", "_bin"))