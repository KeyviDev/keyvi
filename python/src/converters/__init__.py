from .match_iterator_converter import *
from .match_filter_converter import *
from autowrap.ConversionProvider import special_converters


def register_converters():
    special_converters.append(MatchIteratorPairConverter())
    special_converters.append(MatchFilterConverter())
