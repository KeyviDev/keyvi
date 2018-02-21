# -*- coding: utf-8 -*-
# Usage: py.test tests

import keyvi._pycore

import os.path
import subprocess
import sys


def get_package_root():
    module_location = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(keyvi._pycore.__file__)), ".."))

    if (sys.version_info.major >= 3):
        module_location = module_location.encode('utf-8')

    return module_location


def get_interpreter_executable():
    executable = sys.executable

    if (sys.version_info.major >= 3):
        executable = executable.encode('utf-8')

    return executable


def test_merger_binary():
    cmd = get_interpreter_executable() + b" " + os.path.join(get_package_root(), b"_pycore" , b"keyvimerger.py") + b" -h"
    rc = subprocess.call(cmd, shell=True)
    assert rc == 0
