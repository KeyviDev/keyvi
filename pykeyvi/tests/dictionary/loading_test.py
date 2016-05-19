# -*- coding: utf-8 -*-
# Usage: py.test tests

import pykeyvi

import sys
import os

root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(root, "../"))

def test_invalid_filemagic():
    fd = open('broken_file','w')
    fd.write ('dead beef')
    fd.close()
    exception_caught = False
    try:
        d=pykeyvi.Dictionary('broken_file')
    except ValueError:
        exception_caught = True

    assert exception_caught
    os.remove('broken_file')

def test_truncated_file_json():
    c=pykeyvi.JsonDictionaryCompiler()
    c.Add('a', '{1:2}')
    c.Add('b', '{2:4}')
    c.Add('c', '{4:4}')
    c.Add('d', '{2:3}')
    c.Compile()

    c.WriteToFile('truncation_test.kv')
    size = os.path.getsize('truncation_test.kv')

    fd_in = open('truncation_test.kv')
    fd = open('truncation_test1.kv', 'w')
    fd.write(fd_in.read(size/2))
    fd.close()

    exception_caught = False
    try:
        d=pykeyvi.Dictionary('truncation_test1.kv')
    except ValueError:
        exception_caught = True

    assert exception_caught
    os.remove('truncation_test1.kv')
    os.remove('truncation_test.kv')