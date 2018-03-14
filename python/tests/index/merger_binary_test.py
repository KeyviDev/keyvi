# -*- coding: utf-8 -*-
# Usage: py.test tests

from keyvi._core import get_package_root, get_interpreter_executable

import os.path
import subprocess
import sys


def test_merger_binary():
    cmd = get_interpreter_executable() + b" " + os.path.join(get_package_root(), b"_pycore" , b"keyvimerger.py") + b" -h"
    rc = subprocess.call(cmd, shell=True)
    assert rc == 0
