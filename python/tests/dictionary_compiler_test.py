# -*- coding: utf-8 -*-
# Usage: py.test tests

import os
import shutil
import tempfile
import test_tools
from pytest import raises
import gc

from keyvi.compiler import JsonDictionaryCompiler, KeyOnlyDictionaryCompiler

def test_compiler_no_compile_edge_case():
    c = KeyOnlyDictionaryCompiler({"memory_limit_mb":"10"})
    c.add("abc")
    c.add("abd")
    del c


def test_compiler_no_compile_edge_case_empty():
    c = KeyOnlyDictionaryCompiler({"memory_limit_mb":"10"})
    del c


def test_compiler_empty():
    c = KeyOnlyDictionaryCompiler({"memory_limit_mb":"10"})
    with test_tools.tmp_dictionary(c, 'empty.kv') as d:
        assert len(d) == 0

def test_compiler_empty_json():
    c = JsonDictionaryCompiler({"memory_limit_mb":"10"})
    with test_tools.tmp_dictionary(c, 'empty_json.kv') as d:
        assert len(d) == 0


def test_tmp_dir():
    cwd = os.getcwd()
    os.chdir(tempfile.gettempdir())
    try:
        os.mkdir("tmp_dir_test")
        os.chdir(os.path.join(tempfile.gettempdir(), "tmp_dir_test"))
        c = JsonDictionaryCompiler({"memory_limit_mb":"10"})
        c.add("abc", "{'a':2}")
        assert os.listdir('.') == []
        c.compile()
        assert os.listdir('.') == []
        del c
        assert os.listdir('.') == []
    finally:
        os.chdir(cwd)
        os.rmdir(os.path.join(tempfile.gettempdir(), "tmp_dir_test"))


def test_tmp_dir_defined():
    def run_compile(tmpdir):
        c = JsonDictionaryCompiler({"memory_limit_mb":"10", "temporary_path": tmpdir})
        c.add("abc", "{'a':2}")
        c.compile()
        assert os.listdir(tmpdir) != []

    test_dir = os.path.join(tempfile.gettempdir(), "tmp_dir_test_defined")
    try:
        os.mkdir(test_dir)
        run_compile(test_dir)
    finally:
        gc.collect()
        JsonDictionaryCompiler({"memory_limit_mb":"10"})
        shutil.rmtree(test_dir)


def test_compile_step_missing():
    c = KeyOnlyDictionaryCompiler()
    c.add("abc")
    c.add("abd")
    with raises(RuntimeError):
        c.write_to_file("compile_step_missing.kv")


def test_compile_write_to_invalid_file():
    c = KeyOnlyDictionaryCompiler()
    c.add("abc")
    c.add("abd")
    c.compile()
    with raises(ValueError):
        c.write_to_file(os.path.join("invalid", "sub", "directory", "file.kv"))
