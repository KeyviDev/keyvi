# -*- coding: utf-8 -*-
# Usage: py.test tests

import os
import pytest
import sys
import tempfile

from keyvi.dictionary import Dictionary
from keyvi.compiler import JsonDictionaryCompiler

root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(root, "../"))

tmp_dir = tempfile.gettempdir()

def test_invalid_filemagic():
    fd = open(os.path.join(tmp_dir, 'broken_file'),'w')
    fd.write ('dead beef')
    fd.close()
    exception_caught = False
    with pytest.raises(ValueError):
        d=Dictionary(os.path.join(tmp_dir, 'broken_file'))
    os.remove(os.path.join(tmp_dir, 'broken_file'))

def test_non_existing_file():
    assert os.path.exists('non_existing_file') == False
    with pytest.raises(ValueError):
        d=Dictionary(os.path.join(tmp_dir, 'non_existing_file'))

def test_truncated_file_json():
    c=JsonDictionaryCompiler({"memory_limit_mb":"10"})
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

    fd2 = open(os.path.join(tmp_dir,'truncation_test2.kv'), 'wb')
    fd2.write(fd_in.read(int(size-2)))
    fd2.close()

    with pytest.raises(ValueError):
        d=Dictionary(os.path.join(tmp_dir, 'truncation_test1.kv'))
    with pytest.raises(ValueError):
        d=Dictionary(os.path.join(tmp_dir, 'truncation_test2.kv'))
    os.remove(os.path.join(tmp_dir, 'truncation_test2.kv'))
    os.remove(os.path.join(tmp_dir, 'truncation_test1.kv'))
    os.remove(os.path.join(tmp_dir, 'truncation_test.kv'))
