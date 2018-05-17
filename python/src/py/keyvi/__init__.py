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

from keyvi._version import __version__

# global keyvi concepts
from keyvi._core import MatchIterator, Match

# backwards compatibility for 0.2, deprecated, use sub-packages instead

from keyvi._core import CompletionDictionaryCompiler, CompletionDictionaryMerger, Dictionary
from keyvi._core import ForwardBackwardCompletion, FsaTransform, IntDictionaryCompiler, IntDictionaryMerger
from keyvi._core import JsonDictionaryCompiler, JsonDictionaryCompilerSmallData, JsonDictionaryMerger, JumpConsistentHashString
from keyvi._core import KeyOnlyDictionaryCompiler, KeyOnlyDictionaryGenerator, KeyOnlyDictionaryMerger
from keyvi._core import MatchIterator, Match
from keyvi._core import MultiWordCompletion, PredictiveCompression, PrefixCompletion, ReadOnlyIndex
from keyvi._core import StringDictionaryCompiler, StringDictionaryMerger
from keyvi._core import loading_strategy_types
