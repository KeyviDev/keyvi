# -*- coding: utf-8 -*-
# Usage: py.test tests

import pykeyvi

import sys
import os
import tempfile

root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(root, "../"))

tmp_dir = tempfile.gettempdir()

def test_invalid_filemagic():
    fd = open(os.path.join(tmp_dir, 'broken_file'),'w')
    fd.write ('dead beef')
    fd.close()
    exception_caught = False
    try:
        d=pykeyvi.Dictionary(os.path.join(tmp_dir, 'broken_file'))
    except ValueError:
        exception_caught = True

    assert exception_caught
    os.remove(os.path.join(tmp_dir, 'broken_file'))

def test_truncated_file_json():
    c=pykeyvi.JsonDictionaryCompiler()
    c.Add('a', '{1:2}')
    c.Add('b', '{2:4}')
    c.Add('c', '{4:4}')
    c.Add('d', '{2:3}')
    c.Compile()

    c.WriteToFile(os.path.join(tmp_dir,'truncation_test.kv'))
    size = os.path.getsize(os.path.join(tmp_dir, 'truncation_test.kv'))

    fd_in = open(os.path.join(tmp_dir,'truncation_test.kv'), 'rb')
    fd = open(os.path.join(tmp_dir,'truncation_test1.kv'), 'wb')
    fd.write(fd_in.read(int(size/2)))
    fd.close()

    exception_caught = False
    try:
        d=pykeyvi.Dictionary(os.path.join(tmp_dir, 'truncation_test1.kv'))
    except ValueError:
        exception_caught = True

    assert exception_caught
    os.remove(os.path.join(tmp_dir, 'truncation_test1.kv'))
    os.remove(os.path.join(tmp_dir, 'truncation_test.kv'))