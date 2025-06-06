# -*- coding: utf-8 -*-
'''
keyvi - A key value store.

Copyright 2018 Hendrik Muhs<hendrik.muhs@gmail.com>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

  http:www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
'''

from keyvi._core import CompletionDictionaryCompiler, CompletionDictionaryMerger, IntDictionaryCompiler, IntDictionaryMerger
from keyvi._core import JsonDictionaryCompiler, JsonDictionaryMerger
from keyvi._core import KeyOnlyDictionaryCompiler, KeyOnlyDictionaryGenerator, KeyOnlyDictionaryMerger
from keyvi._core import StringDictionaryCompiler, StringDictionaryMerger
from keyvi._core import FloatVectorDictionaryCompiler
from keyvi._core import SecondaryKeyCompletionDictionaryCompiler, SecondaryKeyFloatVectorDictionaryCompiler, SecondaryKeyIntDictionaryCompiler
from keyvi._core import SecondaryKeyKeyOnlyDictionaryCompiler, SecondaryKeyStringDictionaryCompiler, SecondaryKeyJsonDictionaryCompiler

def JsonDictionaryCompilerSmallData(*args, **kwargs):
    from warnings import warn
    warn("JsonDictionaryCompilerSmallData is deprecated and will be removed in a future version. Use JsonDictionaryCompiler instead.")
    return JsonDictionaryCompiler(*args, **kwargs)

def IntDictionaryCompilerSmallData(*args, **kwargs):
    from warnings import warn
    warn("IntDictionaryCompilerSmallData is deprecated and will be removed in a future version. Use IntDictionaryCompiler instead.")
    return IntDictionaryCompiler(*args, **kwargs)
