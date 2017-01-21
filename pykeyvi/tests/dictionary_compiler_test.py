# -*- coding: utf-8 -*-
# Usage: py.test tests

import os
import pykeyvi
import shutil
import tempfile
import test_tools


def test_compiler_no_compile_edge_case():
    c = pykeyvi.KeyOnlyDictionaryCompiler()
    c.Add("abc")
    c.Add("abd")
    del c


def test_compiler_no_compile_edge_case_empty():
    c = pykeyvi.KeyOnlyDictionaryCompiler()
    del c


def test_compiler_empty():
    c = pykeyvi.KeyOnlyDictionaryCompiler()
    with test_tools.tmp_dictionary(c, 'empty.kv') as d:
        assert len(d) == 0

def test_compiler_empty_json():
    c = pykeyvi.JsonDictionaryCompiler()
    with test_tools.tmp_dictionary(c, 'empty_json.kv') as d:
        assert len(d) == 0


def test_tmp_dir():
    cwd = os.getcwd()
    os.chdir(tempfile.gettempdir())
    try:
        os.mkdir("tmp_dir_test")
        os.chdir(os.path.join(tempfile.gettempdir(), "tmp_dir_test"))
        c = pykeyvi.JsonDictionaryCompiler()
        c.Add("abc", "{'a':2}")
        assert os.listdir('.') == []
        c.Compile()
        assert os.listdir('.') == []
        del c
        assert os.listdir('.') == []
    finally:
        os.chdir(cwd)
        os.rmdir(os.path.join(tempfile.gettempdir(), "tmp_dir_test"))


def test_tmp_dir_defined():
    def run_compile(tmpdir):
        c = pykeyvi.JsonDictionaryCompiler({"temporary_path": tmpdir})
        c.Add("abc", "{'a':2}")
        c.Compile()
        assert os.listdir(test_dir) != []

    test_dir = os.path.join(tempfile.gettempdir(), "tmp_dir_test_defined")
    try:
        os.mkdir(test_dir)
        run_compile(test_dir)
    finally:
        pykeyvi.JsonDictionaryCompiler()
        shutil.rmtree(test_dir)
