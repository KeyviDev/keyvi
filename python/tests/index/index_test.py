# -*- coding: utf-8 -*-
# Usage: py.test tests

import keyvi
import os
import shutil
import tempfile

def test_open_index_writer_twice():
    test_dir = os.path.join(tempfile.gettempdir(), "index_writer_test")
    #try:
    #    os.mkdir(test_dir)
    iw1 = keyvi.Index(os.path.join(test_dir,"index"))
    iw1.Set("a", "{}")
    
    iw2 = keyvi.Index(os.path.join(test_dir,"index"))
    iw2.Set("b", "{}")
    #finally:
    #    shutil.rmtree(test_dir)
    