
# import uint32_t type
from libc.stdint cimport uint32_t
from libc.stdint cimport uint64_t
from libc.stdint cimport int32_t
from cpython.version cimport PY_MAJOR_VERSION

from libcpp.string  cimport string as libcpp_utf8_string
from libcpp.vector cimport vector as libcpp_vector
from libcpp.pair  cimport pair  as libcpp_pair
from std_smart_ptr cimport shared_ptr as s_shared_ptr

import json
import msgpack
import keyvi._pycore
import os.path
import sys


def get_package_root():
    module_location = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(keyvi._pycore.__file__)), ".."))

    if (PY_MAJOR_VERSION >= 3):
        module_location = module_location.encode('utf-8')

    return module_location


def get_interpreter_executable():
    executable = sys.executable

    if (PY_MAJOR_VERSION >= 3):
        executable = executable.encode('utf-8')

    return executable
