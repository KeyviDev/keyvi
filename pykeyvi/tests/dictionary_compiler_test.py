# -*- coding: utf-8 -*-
# Usage: py.test tests

import os
import pykeyvi
import shutil
import tempfile

def test_compiler_no_compile_edge_case():
    c = pykeyvi.KeyOnlyDictionaryCompiler()
    c.Add("abc")
    c.Add("abd")
    del c

def test_tmp_dir():
    cwd = os.getcwd()
    try:
        os.mkdir("tmp_dir_test")
        os.chdir(os.path.join(cwd, "tmp_dir_test"))
        c = pykeyvi.JsonDictionaryCompiler()
        c.Add("abc", "{'a':2}")
        assert len(os.listdir('.')) == 0
        c.Compile()
        assert len(os.listdir('.')) == 0
        del c
        assert len(os.listdir('.')) == 0
    finally:
        os.chdir(cwd)
        os.rmdir("tmp_dir_test")


def test_tmp_dir_defined():
    def run_compile(tmpdir):
        c = pykeyvi.JsonDictionaryCompiler(1073741824, {"temporary_path": tmpdir})
        c.Add("abc", "{'a':2}")
        c.Compile()
        assert len(os.listdir(test_dir)) != 0

    try:
        test_dir = "tmp_dir_test_defined"
        os.mkdir(test_dir)
        run_compile(test_dir)
    finally:
        pykeyvi.JsonDictionaryCompiler()
        shutil.rmtree(test_dir)
