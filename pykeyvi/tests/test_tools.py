# -*- coding: utf-8 -*-
# some common tools for tests

import contextlib
import os
import tempfile
import pykeyvi

@contextlib.contextmanager
def tmp_dictionary(compiler, file_name):

    tmp_dir = tempfile.gettempdir()
    fq_file_name = os.path.join(tmp_dir, file_name)
    compiler.Compile()
    compiler.WriteToFile(fq_file_name)
    del compiler
    d = pykeyvi.Dictionary(fq_file_name)
    yield d
    del d
    os.remove(fq_file_name)

def decode_to_unicode(data):
    """Recursively decodes byte strings to unicode"""
    if isinstance(data, bytes):
        return data.decode('utf-8')
    elif isinstance(data, dict):
        return dict((decode_to_unicode(k), decode_to_unicode(v))
            for k, v in data.items())
    elif isinstance(data, list):
        return [decode_to_unicode(e) for e in data]
    return data