
# import uint32_t type
from libc.stdint cimport uint32_t
from libc.stdint cimport uint64_t
from libc.stdint cimport int32_t
from cpython.version cimport PY_MAJOR_VERSION

from libcpp.string  cimport string as libcpp_utf8_string
from libcpp.vector cimport vector as libcpp_vector
from libcpp.pair  cimport pair  as libcpp_pair
from libcpp cimport nullptr

import json
import msgpack
import keyvi._pycore
import os.path
import sys
import warnings


# definition of progress callback for all compilers
cdef void progress_compiler_callback(size_t a, size_t b, void* py_callback) noexcept with gil:
    (<object>py_callback)(a, b)

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

def call_deprecated_method(deprecated_method_name, new_method_name, new_method, *args):
    msg = f"{deprecated_method_name} is deprecated and will be removed in a future version. Use {new_method_name} instead."
    warnings.warn(msg, DeprecationWarning)
    return new_method(*args)

def call_deprecated_method_setter(deprecated_method_name, new_method_name, setter, value):
    msg = f"{deprecated_method_name} is deprecated and will be removed in a future version. Use {new_method_name} instead."
    warnings.warn(msg, DeprecationWarning)
    setter(value)

def call_deprecated_method_getter(deprecated_method_name, new_method_name, getter):
    msg = f"{deprecated_method_name} is deprecated and will be removed in a future version. Use {new_method_name} instead."
    warnings.warn(msg, DeprecationWarning)
    return getter
