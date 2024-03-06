from .match_iterator_converter import *
from autowrap.ConversionProvider import special_converters


def register_converters():
    special_converters.append(MatchIteratorPairConverter())
