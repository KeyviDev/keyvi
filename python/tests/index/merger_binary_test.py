# -*- coding: utf-8 -*-
# Usage: py.test tests

import keyvi._pycore

import os.path
import subprocess
import sys


def get_bin_folder():
    module_location = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(keyvi._pycore.__file__)), ".."))

    if (sys.version_info.major >= 3):
        module_location = module_location.encode('utf-8')

    return os.path.join(module_location, b"_bin")


def test_merger_binary():
    cmd = os.path.join(get_bin_folder(), b"keyvimerger -h")
    rc = subprocess.call(cmd, shell=True)
    assert rc == 0
